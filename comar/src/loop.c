/*
 * COMAR Unified Core - Main Loop & Message Handler
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * ALTYAPI: Modern D-Bus Unified Core, Python 3.12+.
 * [span_4](start_span)GÜVENLİK: Polkit Enforcement & Process Sandboxing.[span_4](end_span)
 */

#include "bus.h"
#include "config.h"
#include "log.h"
#include "process.h"
#include "policy.h"
#include "pydbus.h"
#include "script.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_FDS 1024
#define MAX_PROC 500
#define MAX_WATCHES 10

static struct pollfd bus_fds[MAX_WATCHES];
static DBusWatch *bus_watches[MAX_WATCHES];
static int bus_nr_watches = 0;

/**
 * [span_5](start_span)Kayıtlı Python betiklerini mühürlü bir şekilde icra eder.[span_5](end_span)
 */
void message_execute(DBusMessage *msg, const char *app, const char *model, const char *method) {
    PyObject *py_ret;
    PyObject *py_args = pydbus_import(msg); [span_6](start_span)// Mesajdan veriyi içe aktar[span_6](end_span)
    char *eStr, *vStr;

    [span_7](start_span)// Model ve Metot doğrulaması[span_7](end_span)
    if (validate_model_member(model, method, 0) != 0) {
        bus_reply_unknown_method(msg);
        return;
    }

    [span_8](start_span)// Polkit Yetki Sorgusu (Action ID kontrolü)[span_8](end_span)
    PyObject *model_def = PyDict_GetItemString(PyDict_GetItemString(py_core, "models"), model);
    PyObject *method_def = PyDict_GetItemString(model_def, method);
    char *signature = script_signature(model, method, 1);

    if (method_def != Py_None) {
        const char *action_id = PyUnicode_AsUTF8(PyTuple_GetItem(method_def, 1));
        const char *sender = dbus_message_get_sender(my_proc.bus_msg);

        if (action_id && strcmp(action_id, "") != 0) {
            int result;
            if (policy_check(sender, action_id, &result) == 0) {
                if (result != POLICY_YES) {
                    bus_reply_error(msg, "Comar.PolicyKit", action_id);
                    return;
                }
            } else {
                bus_reply_error(msg, "Comar.PolicyKit", "AuthorizationError");
                return;
            }
        }
    }

    [span_9](start_span)// Metot İcrası[span_9](end_span)
    int exec_res = py_execute(app, model, method, py_args, &py_ret);
    switch (exec_res) {
        case 0: // Başarılı
            bus_reply_object(msg, py_ret, signature);
            break;
        case -1:
        case -2: // Python İstisnası
            py_catch(&eStr, &vStr, 1);
            bus_reply_error(msg, eStr, vStr);
            break;
        default: // Yetki veya İç Hata
            bus_reply_error(msg, "Comar.Internal", "ExecutionFailed");
            break;
    }
}

/**
 * [span_10](start_span)D-Bus mesajını ayrıştırır ve ilgili betiği çatallanmış (fork) süreçte çalıştırır.[span_10](end_span)
 */
void handle_message(DBusMessage *msg) {
    const char *method = dbus_message_get_member(msg);
    const char *iface = dbus_message_get_interface(msg);
    const char *path = dbus_message_get_path(msg);

    log_debug("Süreç Çatallandı: '%s.%s' -> %s\n", iface, method, path);

    if (method == NULL || path == NULL || iface == NULL) {
        bus_reply_unknown_method(msg);
    } else if (strcmp("org.freedesktop.DBus.Introspectable", iface) == 0) {
        message_execute(msg, NULL, "Core", "introspect");
    } else if (strcmp(config_interface, iface) == 0) {
        message_execute(msg, NULL, "Core", method);
    } else if (strncmp(config_interface, iface, strlen(config_interface)) == 0) {
        // Model.Method Ayrıştırması
        char *model = strsub(iface, (int)strlen(config_interface) + 1, (int)strlen(iface));
        char *app = strsub(path, (int)strlen("/package/"), (int)strlen(path));

        message_execute(msg, app, model, method);

        free(model); free(app);
    } else {
        bus_reply_unknown_method(msg);
    }
}

/**
 * [span_11](start_span)Ana süreçte handle edilmesi gereken çekirdek mesajlar (Locale, İptal vb.).[span_11](end_span)
 */
void handle_core_message(DBusMessage *bus_msg, const char *sender, PyObject *py_args) {
    const char *method = dbus_message_get_member(bus_msg);

    if (strcmp(method, "setLocale") == 0) {
        PyDict_SetItemString(PyDict_GetItemString(py_core, "locales"), sender, PyTuple_GetItem(py_args, 0));
        bus_reply_object(bus_msg, Py_True, "b");
    } else if (strcmp(method, "cancel") == 0) {
        int killed = 0;
        for (int i = 0; i < my_proc.nr_children; i++) {
            if (dbus_message_has_sender(my_proc.children[i].bus_msg, sender)) {
                kill(my_proc.children[i].pid, SIGINT);
                killed++;
            }
        }
        bus_reply_object(bus_msg, PyLong_FromLong((long)killed), "i");
    }
}

[span_12](start_span)// ... [D-Bus Watch yönetimi (add_watch, remove_watch, fd_handler) mühürlendi] ...[span_12](end_span)

/**
 * [span_13](start_span)Çekirdek Ana Döngüsü (Main Loop).[span_13](end_span)
 */
int loop_exec() {
    DBusError bus_error;
    dbus_error_init(&bus_error);

    [span_14](start_span)// D-Bus Bağlantısı[span_14](end_span)
    bus_conn = dbus_connection_open_private(config_server_address, &bus_error);
    if (dbus_error_is_set(&bus_error)) {
        log_error("D-Bus bağlantı hatası: %s\n", bus_error.message);
        return -1;
    }

    if (!dbus_bus_register(bus_conn, &bus_error)) {
        log_error("D-Bus kayıt hatası: %s\n", bus_error.message);
        return -1;
    }

    [span_15](start_span)// Unified İsim İsteği (tr.org.pardus.comar)[span_15](end_span)
    dbus_bus_request_name(bus_conn, config_service_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &bus_error);
    log_info("Kayıtlı Servis İsmi: '%s'\n", config_service_name);

    dbus_connection_set_watch_functions(bus_conn, add_watch, remove_watch, NULL, NULL, NULL);
    dbus_connection_add_filter(bus_conn, filter_func, NULL, NULL);

    while (1) {
        struct pollfd fds[MAX_FDS];
        int nr_fds = 0;

        [span_16](start_span)// D-Bus Watch'larını poll listesine ekle[span_16](end_span)
        for (int i = 0; i < bus_nr_watches; i++) {
            if (bus_fds[i].fd == 0 || !dbus_watch_get_enabled(bus_watches[i])) continue;
            fds[nr_fds] = bus_fds[i];
            nr_fds++;
        }

        [span_17](start_span)// Alt süreç (çocuk) borularını izle[span_17](end_span)
        int watch_limit = nr_fds;
        for (int i = 0; i < my_proc.nr_children; i++) {
            fds[nr_fds].fd = my_proc.children[i].from;
            fds[nr_fds].events = POLLIN;
            nr_fds++;
        }

        int poll_res = poll(fds, nr_fds, config_timeout > 0 ? config_timeout * 1000 : -1);
        if (poll_res < 0) {
            if (errno == EINTR) continue;
            return -1;
        }

        [span_18](start_span)// Olayları İşle[span_18](end_span)
        for (int i = 0; i < nr_fds; i++) {
            if (fds[i].revents == 0) continue;
            if (i < watch_limit) {
                fd_handler(fds[i].revents, bus_watches[i]);
            } else {
                [span_19](start_span)// Alt süreç tamamlanmış, envanterden temizle[span_19](end_span)
                for (int j = 0; j < my_proc.nr_children; j++) {
                    if (my_proc.children[j].from == fds[i].fd) {
                        rem_child(j);
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
