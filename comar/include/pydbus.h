/*
 * COMAR Python D-Bus Bridge (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: Python 3.12+ C-API entegrasyonu.
 * GÜVENLİK: API Çağrı Mühürleme ve Variant Doğrulama.
 * STANDART: D-Bus 1.12+, Asyncio uyumlu veri dönüşümü.
 */

#ifndef PYDBUS_H
#define PYDBUS_H

#include <Python.h>
#include <dbus/dbus.h>
#include <stdbool.h>

/* --- 2026 Tip Tanımları --- */
/* 'v' (Variant) desteği TODO listesi uyarınca eklendi. */
#define TYPES_BASIC "sbixd"
#define TYPES_CONTAINER "arDv" 

/**
 * @brief Python nesnesinin D-Bus imzasını (signature) döndürür.
 * 2026 Protokolü: Introspection verileriyle tam uyumlu çalışır.
 */
char *get_obj_sign(PyObject *obj);

/* --- İhracat (Python -> D-Bus) --- */

/**
 * @brief Python nesnesini D-Bus mesajına ihraç eder.
 * @return Başarıda 1, dönüşüm hatasında istisna fırlatır ve 0 döner.
 */
int pydbus_export(DBusMessageIter *iter, PyObject *obj, char *signature);
int pydbus_export_item(DBusMessageIter *iter, PyObject *obj, char *signature);

/* --- İthalat (D-Bus -> Python) --- */

/**
 * @brief D-Bus mesajını Python nesnesine dönüştürür.
 * 2026 Protokolü: Asenkron API katmanına mühürlü veri iletir.
 */
PyObject *pydbus_import(DBusMessage *msg);
PyObject *py_get_item(DBusMessageIter* iter);
PyObject *py_get_dict(DBusMessageIter *iter);
PyObject *py_get_tuple(DBusMessageIter *iter);
PyObject *py_get_list(DBusMessageIter *iter);

/**
 * @brief Variant tipindeki veriyi Python nesnesine dönüştürür.
 * Bu fonksiyon COMAR 2.0 esnek veri yapısı için kritiktir.
 */
PyObject *py_get_variant(DBusMessageIter *iter);

/**
 * @brief ZEKA: Dönüşüm hatalarında teknisyen dostu raporlama yapar.
 */
void report_conversion_error(const char *expected, PyObject *received);

#endif /* PYDBUS_H */
