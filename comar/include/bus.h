/*
 * COMAR D-Bus Communication Bridge (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: Python 3.12+ C-API, Asyncio entegrasyonu.
 * GÜVENLİK: BLAKE3 Çağrı Mühürleme ve Polkit 0.96.
 * SİSTEM: Bloklamayan (Non-blocking) asenkron D-Bus trafiği.
 */

#ifndef BUS_H
#define BUS_H

#include <Python.h>
#include <dbus/dbus.h>
#include <stdbool.h>

/* --- Merkezi Bağlantı Nesnesi --- */
extern DBusConnection *bus_conn;

/* --- 2026 Protokol Fonksiyonları --- */

/**
 * @brief D-Bus bağlantısını başlatır ve sinyal filtrelerini kurar.
 * @return Başarıda true.
 */
bool bus_init(void);

/**
 * @brief D-Bus mesajını kuyruğa ekler ve gönderir.
 */
void bus_send(DBusMessage *bus_msg);

/* --- Yanıt Mekanizmaları --- */

/**
 * @brief Hata yanıtlarını ZEKA (AI Analiz) bağlamıyla birlikte gönderir.
 * @param bus_msg Yanıtlanacak orijinal mesaj.
 * @param name D-Bus hata adı.
 * @param msg Teknisyen dostu hata detayı.
 */
void bus_reply_error(DBusMessage *bus_msg, const char *name, const char *msg);

/**
 * @brief Python nesnesini D-Bus objesi olarak mühürlü paketleyip yanıtlar.
 */
void bus_reply_object(DBusMessage *bus_msg, PyObject *py_obj, char *signature);

/**
 * @brief Yetki reddedildiğinde Polkit 0.96 uyumlu hata döner.
 */
void bus_reply_unauthorized(DBusMessage *bus_msg, const char *action);

/**
 * @brief Bilinmeyen metot çağrılarını standart hata ile yanıtlar.
 */
void bus_reply_unknown_method(DBusMessage *bus_msg);

/* --- Sinyal ve Çağrı Yönetimi --- */

/**
 * @brief Sistem genelinde mühürlü bir sinyal yayınlar (broadcast).
 * @return Başarıda 1.
 */
int bus_signal(const char *path, const char *interface, const char *member, PyObject *obj, char *signature);

/**
 * @brief Bir API çağrısının BLAKE3 mühürünü doğrular.
 * Dışarıdan gelen manipüle edilmiş D-Bus paketlerini engellemek için kritiktir.
 * @return Mühür geçerliyse true.
 */
bool bus_verify_call_seal(DBusMessage *msg);

/**
 * @brief MODERN: Asenkron D-Bus çağrısı yapar (Bloklamayan).
 * Python tarafındaki Asyncio döngüsüyle paralel çalışır.
 * @return Python Future/Object sonucu.
 */
PyObject *bus_call_async(const char *path, const char *interface, const char *member, PyObject *obj, int timeout, char *signature);

/**
 * @brief Bir D-Bus metodunu mühürlü ve asenkron olarak icra eder.
 */
PyObject *bus_execute_sealed(DBusConnection *conn, const char *path, const char *interface, const char *member, PyObject *obj, int timeout, char *signature);

#endif /* BUS_H */
