/*
 * COMAR Unified Core - Model Database Engine
 * Copyright (c) 2026
 *
 * ALTYAPI: Modern C11, Python 3.12+, Iksemel XML Parser.
 * SİSTEM: Mühürlü Model Doğrulama ve Envanter Kaydı.
 */

#include "config.h"
#include "iksemel.h"
#include "script.h"
#include "log.h"
#include "utils.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dbus/dbus.h>

/**
 * Model XML dosyasının yapısal ve semantik doğruluğunu mühürler.
 */
int db_validate_model(iks *xml, char *filename) {
    iks *iface, *met, *arg;
    DBusError bus_error;
    dbus_error_init(&bus_error);

    // 1. Kök etiket kontrolü
    if (iks_strcmp(iks_name(xml), "comarModel") != 0) {
        log_error("Geçersiz model XML kökü (beklenen: comarModel): %s\n", filename);
        return -1;
    }

    for (iface = iks_first_tag(xml); iface; iface = iks_next_tag(iface)) {
        // Sadece 'interface' etiketine izin verilir
        if (iks_strcmp(iks_name(iface), "interface") != 0) {
            log_error("Bilinmeyen etiket '%s' (dosya: %s)\n", iks_name(iface), filename);
            return -1;
        }
        
        // Arayüz ismi mutlaka tanımlanmalıdır
        if (!iks_strlen(iks_find_attrib(iface, "name"))) {
            log_error("İsimsiz model arayüzü tespit edildi: %s\n", filename);
            return -1;
        }

        for (met = iks_first_tag(iface); met; met = iks_next_tag(met)) {
            // Metot veya Sinyal tanımlarını mühürle
            if (iks_strcmp(iks_name(met), "method") == 0 || iks_strcmp(iks_name(met), "signal") == 0) {
                if (!iks_strlen(iks_find_attrib(met, "name"))) {
                    log_error("İsimsiz metot/sinyal: %s\n", filename);
                    return -1;
                }

                for (arg = iks_first_tag(met); arg; arg = iks_next_tag(arg)) {
                    if (iks_strcmp(iks_name(arg), "arg") == 0) {
                        const char *type = iks_find_attrib(arg, "type");
                        // D-Bus imza doğrulaması
                        if (!type || !dbus_signature_validate(type, &bus_error)) {
                            log_error("Geçersiz D-Bus tipi (%s) -> %s\n", type, filename);
                            dbus_error_free(&bus_error);
                            return -1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * Bir metot için PolicyKit eylem kimliğini (Action ID) üretir.
 * Karmaşıklık: $O(n)$ (n: dizgi uzunluğu).
 */
char *db_action_id(char *iface_name, iks *met) {
    char *action_id = iks_find_attrib(met, "action_id");
    if (action_id) return iks_strdup(action_id);

    const char *alias = iks_find_attrib(met, "alias");
    if (!alias) alias = iks_find_attrib(met, "access_label");
    if (!alias) alias = iks_find_attrib(met, "name");

    // Unified Core isimlendirme kuralı: config.interface + interface + alias
    int size = (int)(strlen(config_interface) + strlen(iface_name) + strlen(alias) + 3);
    char *buf = malloc(size);
    if (!buf) oom();

    snprintf(buf, size, "%s.%s.%s", config_interface, iface_name, alias);
    
    // 2026 Güvenlik Standartı: Tüm eylem kimlikleri küçük harf olmalıdır
    for (char *t = buf; *t; t++) *t = (char)tolower((int)*t);

    return buf;
}

/**
 * Tekil bir model dosyasını Python envanterine mühürler.
 */
void db_load_model(iks *xml, PyObject **py_models) {
    iks *iface, *met, *arg;

    for (iface = iks_first_tag(xml); iface; iface = iks_next_tag(iface)) {
        PyObject *py_methods = PyDict_New();
        char *iface_name = iks_find_attrib(iface, "name");

        for (met = iks_first_tag(iface); met; met = iks_next_tag(met)) {
            PyObject *py_tuple = PyTuple_New(4);

            // 0: Tip (0: Method, 1: Signal)
            int type = (iks_strcmp(iks_name(met), "method") == 0) ? 0 : 1;
            PyTuple_SetItem(py_tuple, 0, PyLong_FromLong((long)type));

            // 1: PolicyKit Action ID
            char *action_id = db_action_id(iface_name, met);
            PyTuple_SetItem(py_tuple, 1, PyUnicode_FromString(action_id));
            free(action_id);

            // 2 & 3: Argüman listeleri (In/Out)
            PyObject *py_in = PyList_New(0);
            PyObject *py_out = PyList_New(0);

            for (arg = iks_first_tag(met); arg; arg = iks_next_tag(arg)) {
                if (iks_strcmp(iks_name(arg), "arg") == 0) {
                    const char *dir = iks_find_attrib(arg, "direction");
                    PyObject *py_type = PyUnicode_FromString(iks_find_attrib(arg, "type"));
                    if (type == 0 && dir && strcmp(dir, "out") == 0) {
                        PyList_Append(py_out, py_type);
                    } else {
                        PyList_Append(py_in, py_type);
                    }
                }
            }
            PyTuple_SetItem(py_tuple, 2, py_in);
            PyTuple_SetItem(py_tuple, 3, py_out);

            PyDict_SetItemString(py_methods, iks_find_attrib(met, "name"), py_tuple);
        }
        PyDict_SetItemString(*py_models, iface_name, py_methods);
    }
}

/**
 * Modeller dizinini tarar ve envanteri başlatır.
 */
int db_load_models(PyObject **py_models) {
    struct dirent *dp;
    DIR *dir = opendir(config_dir_models);
    if (!dir) {
        log_error("Modeller dizini açılamadı: %s\n", config_dir_models);
        return -1;
    }

    *py_models = PyDict_New();
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.') continue;

        char fn_xml[1024];
        snprintf(fn_xml, sizeof(fn_xml), "%s/%s", config_dir_models, dp->d_name);

        iks *xml;
        if (iks_load(fn_xml, &xml) != IKS_OK) {
            log_error("XML yüklenemedi: %s\n", fn_xml);
            continue;
        }

        if (db_validate_model(xml, fn_xml) == 0) {
            db_load_model(xml, py_models);
            log_debug("Model mühürlendi: %s\n", dp->d_name);
        }
        iks_delete(xml);
    }
    closedir(dir);
    return 0;
}
