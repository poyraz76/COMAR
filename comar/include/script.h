/*
 * COMAR Script Execution Engine (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: Python 3.12+ C-API, Asyncio entegrasyonu.
 * GÜVENLİK: BLAKE3 Betik Doğrulama (Seal Enforcement).
 * SİSTEM: Systemd-free, Müdür + COMAR uyumlu.
 */

#ifndef SCRIPT_H
#define SCRIPT_H

#include <Python.h>
#include <stdbool.h>

/* --- Merkezi Nesneler --- */
extern PyObject *py_core;

/* --- Modernize Edilmiş COMAR İstisnaları --- */
extern PyObject *PyExc_COMAR_Internal;
extern PyObject *PyExc_COMAR_Invalid;
extern PyObject *PyExc_COMAR_Script;
extern PyObject *PyExc_COMAR_Missing;
extern PyObject *PyExc_COMAR_Seal;      /* Yeni: BLAKE3 mühür ihlali hatası */
extern PyObject *PyExc_DBus;
extern PyObject *PyExc_PolicyKit;

/* --- Yaşam Döngüsü Yönetimi --- */

/**
 * @brief Python yorumlayıcısını ve Asyncio döngüsünü hazırlar.
 */
int script_init(void);

/**
 * @brief Kaynakları temizler ve Python oturumunu kapatır.
 */
void script_finalize(void);

/* --- İcra ve Hata Yönetimi --- */

/**
 * @brief Oluşan Python hatalarını yakalar ve ZEKA (AI Analiz) sistemine raporlar.
 */
void py_catch(char **eStr, char **vStr, int log);

/**
 * @brief Bir betiği çalıştırmadan önce BLAKE3 mühürünü doğrular.
 * @return Mühür geçerliyse true, ihlal varsa false.
 */
bool script_verify_seal(const char *path);

/**
 * @brief Belirtilen metodu asenkron uyumlu şekilde icra eder.
 * 2026 kuralı: İcra öncesi mühür kontrolü zorunludur.
 */
int py_execute(const char *str_application, const char *str_model, const char *str_method, PyObject *py_args, PyObject **py_ret);

/* --- Model ve İmza Denetimi --- */

/**
 * @brief Model üyelerinin (metotlar) geçerliliğini denetler.
 */
int validate_model_member(const char *model, const char *method, int type);

/**
 * @brief D-Bus Introspection için metot imzalarını (signature) döndürür.
 */
char *script_signature(const char *model, const char *member, int direction);

/**
 * @brief İmzayı Python nesnesi olarak ayrıştırır.
 */
PyObject *script_signature_each(const char *signature);

/* --- Otonom Görev Tetikleyicileri (TODO listesi uyarınca) --- */

/**
 * @brief Kayıt sonrası onRegister metotlarını tetikler.
 */
int script_trigger_register(const char *package);

#endif // SCRIPT_H
