/*
 * COMAR Database Access Layer (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * SİSTEM: Python 3.12+ C-API entegrasyonu.
 * GÜVENLİK: BLAKE3 mühür kontrolü ve SQLite bütünlüğü.
 * MERKEZ: /var/lib/pisi/inventory.db erişim katmanı.
 */

#ifndef DB_H
#define DB_H

#include <Python.h>
#include <sqlite3.h>
#include <stdbool.h>

/* --- 2026 Standartları --- */
#define COMAR_DB_PATH "/var/lib/pisi/inventory.db"

/**
 * @brief Merkezi envanter veritabanını başlatır.
 * @return SQLITE_OK veya hata kodu.
 */
int db_init(void);

/**
 * @brief SQLite üzerinden kayıtlı modelleri Python nesnesi olarak yükler.
 * @param py_models Modellerin yükleneceği Python Dictionary objesi.
 * @return Başarı durumunda 0, hata durumunda -1.
 */
int db_load_models(PyObject **py_models);

/**
 * @brief Kayıtlı bir betiğin (script) BLAKE3 mühürünü envanterden doğrular.
 * @param path Betiğin dosya yolu.
 * @param hash Envanterdeki beklenen mühür.
 */
int db_verify_seal(const char *path, const char *hash);

/**
 * @brief Yeni bir uygulamayı merkezi envantere mühürlü olarak kaydeder.
 * @return Başarı durumunda 0.
 */
int db_register_app(const char *package, const char *model, const char *script_path);

/* --- 2026 MODERN EKLEMELER --- */

/**
 * @brief MODERN: Veritabanı Şeması Versiyon Kontrolü.
 * setup.py tarafından belirlenen versiyonla uyumluluğu denetler.
 */
int db_check_schema_version(const char *expected_version);

/**
 * @brief MODERN: Atomik İşlem Yönetimi (Transaction).
 * Toplu kayıt işlemlerinde (örn: toplu mühürleme) veri bütünlüğünü korur.
 */
int db_begin_transaction(void);
int db_commit_transaction(void);
int db_rollback_transaction(void);

/**
 * @brief MODERN: ZEKA (AI Analiz) Denetim Kaydı (Audit Log).
 * Kritik DB değişikliklerini doğrudan 'event_logs' tablosuna mühürler.
 */
int db_audit_log(const char *module, const char *event, const char *details);

/**
 * @brief Veritabanı bağlantısını güvenli bir şekilde kapatır.
 */
void db_finalize(void);

#endif // DB_H
