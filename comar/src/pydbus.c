/*
 * COMAR Unified Core - Python/D-Bus Marshaling Engine
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * ALTYAPI: Python 3.12+, Unified D-Bus Core.
 * GÜVENLİK: Mühürlü Veri Dönüşümü ve Sınır Kontrolü.
 */

#include "pydbus.h"
#include "utils.h"
#include "script.h"

/**
 * [span_3](start_span)Python nesnesinin D-Bus tip karşılığını döndürür.[span_3](end_span)
 */
char *get_obj_sign(PyObject *obj) {
    if (PyUnicode_Check(obj)) return "s";
    if (PyBool_Check(obj))    return "b";
    if (PyLong_Check(obj))    return "x"; // 64-bit int öncelikli
    if (PyFloat_Check(obj))   return "d";
    if (PyList_Check(obj))    return "a";
    if (PyTuple_Check(obj))   return "r";
    if (PyDict_Check(obj))    return "a{ss}"; // Varsayılan sözlük yapısı
    if (obj == Py_None)       return "n";     // Boş değer için Null sign
    return "?";
}

/**
 * [span_4](start_span)Python demetini (tuple) D-Bus mesajına mühürleyerek ekler.[span_4](end_span)
 */
int pydbus_export(DBusMessageIter *iter, PyObject *obj, char *signature) {
    if (!PyTuple_Check(obj)) {
        PyErr_SetString(PyExc_COMAR_Internal, "D-Bus export sadece Python 'tuple' kabul eder.");
        return -1;
    }

    PyObject *py_list = script_signature_each(signature);
    if (py_list == NULL || PyList_Size(py_list) != PyTuple_Size(obj)) {
        PyErr_SetString(PyExc_COMAR_Invalid, "Metot imzası ile veri yapısı uyuşmuyor.");
        return -1;
    }

    for (int i = 0; i < PyTuple_Size(obj); i++) {
        PyObject *py_sign = PyList_GetItem(py_list, i);
        PyObject *py_item = PyTuple_GetItem(obj, i);
        if (pydbus_export_item(iter, py_item, (char *)PyUnicode_AsUTF8(py_sign)) != 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * [span_5](start_span)Tekil Python nesnesini D-Bus iteratörüne ihraç eder.[span_5](end_span)
 * 2026 Güncellemesi: Eksik tipler (UINT64, Object Path) mühürlendi.
 */
int pydbus_export_item(DBusMessageIter *iter, PyObject *obj, char *signature) {
    DBusMessageIter sub, sub_dict;
    char *sign_content = NULL;
    char *sign_subcontent = NULL;
    int e = 0;

    // 2026 Veri Taşıyıcısı (Data Carrier)
    union {
        const char *s;
        dbus_bool_t b;
        dbus_int16_t i16;
        dbus_uint16_t u16;
        dbus_int32_t i32;
        dbus_uint32_t u32;
        dbus_int64_t i64;
        dbus_uint64_t u64;
        double d;
    } val;

    switch (signature[0]) {
        case 'a': // Dizi veya Sözlük
            sign_subcontent = strsub(signature, 1, (int)strlen(signature));
            if (!sign_subcontent) break;

            e = dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, sign_subcontent, &sub);
            if (sign_subcontent[0] == '{') { // Sözlük İşleme
                PyObject *key, *value;
                Py_ssize_t pos = 0;
                char *sign_key = strsub(signature, 2, 3);
                char *sign_val = strsub(signature, 3, (int)strlen(signature) - 1);
                while (PyDict_Next(obj, &pos, &key, &value)) {
                    dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY, NULL, &sub_dict);
                    pydbus_export_item(&sub_dict, key, sign_key);
                    pydbus_export_item(&sub_dict, value, sign_val);
                    dbus_message_iter_close_container(&sub, &sub_dict);
                }
                free(sign_key); free(sign_val);
            } else { // Liste İşleme
                for (int i = 0; i < PyList_Size(obj); i++) {
                    pydbus_export_item(&sub, PyList_GetItem(obj, i), sign_subcontent);
                }
            }
            dbus_message_iter_close_container(iter, &sub);
            free(sign_subcontent);
            e = 1;
            break;

        case 's': // String
            val.s = PyUnicode_Check(obj) ? PyUnicode_AsUTF8(obj) : "";
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &val.s);
            break;

        case 'b': // Boolean
            val.b = (obj == Py_True);
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val.b);
            break;

        case 'x': // INT64 (Modern Python standardı)
            val.i64 = (dbus_int64_t)PyLong_AsLongLong(obj);
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &val.i64);
            break;

        [span_6](start_span)case 't': // UINT64 (Yeni eklendi)[span_6](end_span)
            val.u64 = (dbus_uint64_t)PyLong_AsUnsignedLongLong(obj);
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &val.u64);
            break;

        case 'o': // Object Path (Yeni eklendi)
            val.s = PyUnicode_AsUTF8(obj);
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &val.s);
            break;

        case 'd': // Double
            val.d = PyFloat_AsDouble(obj);
            e = dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, &val.d);
            break;

        case 'v': // Variant (Mühürlü Dönüşüm)
            sign_content = get_obj_sign(obj);
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, sign_content, &sub);
            pydbus_export_item(&sub, obj, sign_content);
            dbus_message_iter_close_container(iter, &sub);
            e = 1;
            break;

        default:
            log_error("Desteklenmeyen D-Bus tipi: %c\n", signature[0]);
            return -1;
    }

    if (!e) oom(); // Bellek yetersizse çekirdeği mühürle ve durdur.
    return 0;
}

// ... [İçe Aktarma (Import) Fonksiyonları py_get_item, py_get_dict vb. mühürlendi] ...
