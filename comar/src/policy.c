/*
 * COMAR Unified Core - PolicyKit Authority Engine
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * ALTYAPI: Polkit 1.0 (Authority Interface), Unified D-Bus Core.
 * GÜVENLİK: UID 0 (Root) Bypass & Mühürlü Yetki Sorgusu.
 */

#include "log.h"
#include "bus.h"
#include "script.h"
#include "policy.h"

/**
 * Gönderenin belirtilen eylemi (action) gerçekleştirmeye yetkili olup olmadığını kontrol eder.
 * @sender: Çağrıyı yapanın D-Bus ismi.
 * @action: Polkit eylem kimliği (Örn: tr.org.pardus.comar.net.stack.set).
 * @result: Sonuç (POLICY_YES / POLICY_NO).
 * @return: Başarı durumunda 0, hata durumunda -1 döner.
 */
int policy_check(const char *sender, const char *action, int *result) {
    DBusConnection *conn;
    DBusError err;
    int uid = -1;

    // Varsayılan olarak yetki reddedilir.
    *result = POLICY_NO;

    dbus_error_init(&err);

    // Sistem Bus'ına bağlan.
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        log_error("PolicyKit sorgusu için D-Bus bağlantısı kurulamadı: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }

    // Çağrıyı yapanın UID bilgisini al.
    uid = dbus_bus_get_unix_user(conn, sender, &err);
    if (dbus_error_is_set(&err)) {
        log_error("Çağrıyı yapanın UID bilgisi alınamadı: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }

    // 2026 Protokolü: Root kullanıcısı (UID 0) her zaman yetkilidir, Polkit'e sorma.
    if (uid == 0) {
        log_debug("Root yetkisi tespit edildi, Polkit sorgusu atlanıyor.\n");
        *result = POLICY_YES;
        return 0;
    }

    // Polkit Sorgu Objelerini Hazırla (Python 3.12 Interop)
    // Subject: (system-bus-name, {name: sender})
    PyObject *subject = PyTuple_New(2);
    PyTuple_SetItem(subject, 0, PyUnicode_FromString("system-bus-name"));
    PyObject *subject_details = PyDict_New();
    PyDict_SetItemString(subject_details, "name", PyUnicode_FromString(sender));
    PyTuple_SetItem(subject, 1, subject_details);

    PyObject *details = PyDict_New(); // Ek detaylar (genellikle boş)

    // Polkit.CheckAuthorization Parametreleri
    PyObject *obj = PyTuple_New(5);
    PyTuple_SetItem(obj, 0, subject);
    PyTuple_SetItem(obj, 1, PyUnicode_FromString(action));
    PyTuple_SetItem(obj, 2, details);
    PyTuple_SetItem(obj, 3, PyLong_FromLong((long) 1)); // AllowUserInteraction: True
    PyTuple_SetItem(obj, 4, PyUnicode_FromString("comar-auth-request")); // Cancellation ID

    // Polkit Authority servisine mühürlü çağrı yap.
    PyObject *ret = bus_execute2(conn, 
                                "org.freedesktop.PolicyKit1", 
                                "/org/freedesktop/PolicyKit1/Authority", 
                                "org.freedesktop.PolicyKit1.Authority", 
                                "CheckAuthorization", 
                                obj, -1, "(sa{sv})sa{ss}us");

    if (!ret) {
        char *eStr, *vStr;
        py_catch(&eStr, &vStr, 1);
        log_error("Polkit sorgusu başarısız: %s - %s\n", eStr, vStr);
        *result = POLICY_NO;
        return 0;
    }

    // Sonucu Ayrıştır (Polkit AuthorizationResult döner)
    // ret[0] = is_authorized (boolean)
    if (PyTuple_GetItem(ret, 0) == Py_True) {
        log_debug("Polkit onayı alındı: %s için %s eylemi.\n", sender, action);
        *result = POLICY_YES;
    } else {
        log_info("Polkit yetkiyi reddetti: %s için %s eylemi.\n", sender, action);
    }

    return 0;
}
