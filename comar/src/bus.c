/*
 * COMAR Unified Core - D-Bus Communication Engine
 * Copyright (c) 2026
 *
 * ALTYAPI: Modern D-Bus Lib, Python 3.12+.
 * GÜVENLİK: Bellek Güvenliği ve Mühürlü Mesajlaşma.
 */

#include "config.h"
#include "log.h"
#include "process.h"
#include "pydbus.h"
#include "script.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>

// Küresel D-Bus Bağlantısı
DBusConnection *bus_conn;

/**
 * D-Bus mesajını gönderir ve hattı (connection) temizler.
 */
void bus_send(DBusMessage *bus_msg) {
    dbus_uint32_t serial = 0;
    if (bus_msg) {
        dbus_connection_send(bus_conn, bus_msg, &serial);
        dbus_connection_flush(bus_conn);
    }
}

/**
 * Hatalı metot çağrılarına mühürlü bir hata mesajı ile yanıt döner.
 */
void bus_reply_error(DBusMessage *bus_msg, const char *name, const char *msg) {
    // 2026 Güvenlik Kuralı: Hata isimleri arayüz önekiyle birleştirilir.
    char exception[512];
    snprintf(exception, sizeof(exception), "%s.%s", config_interface, name);

    DBusMessage *error = dbus_message_new_error(bus_msg, exception, msg);
    bus_send(error);
    dbus_message_unref(error);
}

/**
 * Python nesnesini D-Bus mesajına dönüştürerek yanıt verir.
 */
void bus_reply_object(DBusMessage *bus_msg, PyObject *py_obj, char *signature) {
    DBusMessage *reply;
    DBusMessageIter iter;

    reply = dbus_message_new_method_return(bus_msg);
    dbus_message_iter_init_append(reply, &iter);

    // İmza mevcutsa veriyi D-Bus formatına mühürle
    if (signature && strcmp(signature, "") != 0) {
        PyObject *py_tuple;
        if (!PyTuple_Check(py_obj)) {
            py_tuple = PyTuple_New(1);
            PyTuple_SetItem(py_tuple, 0, py_obj);
            Py_INCREF(py_obj); // Referans sayısını koru
        } else {
            py_tuple = py_obj;
        }

        if (pydbus_export(&iter, py_tuple, signature) != 0) {
            char *eStr, *vStr;
            py_catch(&eStr, &vStr, 1);
            bus_reply_error(bus_msg, eStr, vStr);
            return;
        }
    }

    bus_send(reply);
    dbus_message_unref(reply);
}

/**
 * D-Bus sinyallerini mühürlü bir şekilde yayınlar (Emit).
 */
int bus_signal(const char *path, const char *interface, const char *member, PyObject *obj, char *signature) {
    DBusMessage *msg;
    DBusMessageIter iter;

    msg = dbus_message_new_signal(path, interface, member);
    if (!msg) return -1;

    dbus_message_iter_init_append(msg, &iter);

    if (obj && obj != Py_None && signature && strcmp(signature, "") != 0) {
        PyObject *py_tuple = PyTuple_Check(obj) ? obj : Py_BuildValue("(O)", obj);
        if (pydbus_export(&iter, py_tuple, signature) != 0) {
            dbus_message_unref(msg);
            return -1;
        }
    }

    bus_send(msg);
    dbus_message_unref(msg);
    return 0;
}

/**
 * Senkron (blocking) metot çağrısı yapar ve yanıtı Python nesnesi olarak döner.
 */
PyObject *bus_execute2(DBusConnection *conn, const char *destination, const char *path, const char *interface, const char *member, PyObject *obj, int timeout, char *signature) {
    DBusMessage *msg, *reply;
    DBusMessageIter iter;
    DBusError err;

    msg = dbus_message_new_method_call(destination, path, interface, member);
    dbus_message_iter_init_append(msg, &iter);

    if (signature && strcmp(signature, "") != 0) {
        PyObject *py_tuple = PyTuple_Check(obj) ? obj : Py_BuildValue("(O)", obj);
        if (pydbus_export(&iter, py_tuple, signature) != 0) {
            dbus_message_unref(msg);
            return NULL;
        }
    }

    // Zaman aşımı ayarı (milisaniye cinsinden)
    int dbus_timeout = (timeout == -1) ? -1 : timeout * 1000;

    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block(conn, msg, dbus_timeout, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        PyErr_Format(PyExc_DBus, "Metot çağrısı başarısız: %s", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    PyObject *ret = NULL;
    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
        ret = pydbus_import(reply); // D-Bus'tan Python'a içe aktar
        if (ret && PyTuple_Check(ret) && PyTuple_Size(ret) == 1) {
            PyObject *tmp = PyTuple_GetItem(ret, 0);
            Py_INCREF(tmp);
            Py_DECREF(ret);
            ret = tmp;
        }
    }
    dbus_message_unref(reply);
    return ret ? ret : Py_None;
}

/**
 * Belirli bir dil seçeneği ile (locale) metot çağrısı yapar.
 */
PyObject *bus_call(const char *path, const char *interface, const char *member, PyObject *obj, int timeout, const char *lang, char *signature) {
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        PyErr_SetString(PyExc_DBus, "Çağrı için özel D-Bus bağlantısı kurulamadı.");
        dbus_error_free(&err);
        return NULL;
    }

    // Önce yerel dil ayarını mühürle
    PyObject *args = Py_BuildValue("(s)", lang);
    PyObject *ret = bus_execute2(conn, config_service_name, "/", config_interface, "setLocale", args, 25, "s");
    Py_DECREF(args);

    if (ret) {
        // Asıl metodu icra et
        ret = bus_execute2(conn, config_service_name, path, interface, member, obj, timeout, signature);
    }

    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    return ret;
}
