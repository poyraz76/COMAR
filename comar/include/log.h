/*
 * COMAR Logging System (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: C11, Zstd Sıkıştırmalı Arşivleme.
 * GÜVENLİK: Mühürlü Günlük Kaydı (Sealed Logging).
 * SİSTEM: ZEKA (AI Analiz) entegrasyonu.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>

/* --- 2026 Standartları --- */
#define LOG_DIR "/var/log/comar"
#define LOG_DB_PATH "/var/lib/pisi/inventory.db" /* Olayların mühürlendiği yer */

/* --- Günlük Seviyeleri --- */

/**
 * @brief Kritik sistem hatalarını mühürlü olarak kaydeder.
 */
void log_error(const char *fmt, ...);

/**
 * @brief Sistem uyarılarını (mühür ihlali denemeleri vb.) kaydeder.
 */
void log_warning(const char *fmt, ...);

/**
 * @brief Genel sistem bilgilerini ve servis durum değişikliklerini kaydeder.
 */
void log_info(const char *fmt, ...);

/**
 * @brief Geliştirici ve teknisyen seviyesinde detaylı izleme yapar.
 */
void log_debug(const char *fmt, ...);

/* --- 2026 Protokol Fonksiyonları --- */

/**
 * @brief ZEKA: Teknisyen dostu hata analizi ve çözüm önerisi sunar.
 * Bu fonksiyon Python tarafındaki ai_analyze_utility ile senkronize çalışır.
 * * @param context Hatanın oluştuğu sistem bağlamı.
 * @param technical_tip Çözüm için teknisyen ipucu (Poyraz76 standartları).
 */
void log_technical_tip(const char *context, const char *technical_tip);

/**
 * @brief Günlük sistemini başlatır ve Zstd arşiv motorunu hazırlar.
 * @return Başarıda true.
 */
bool log_init(void);

/**
 * @brief Günlük sistemini kapatır ve bekleyen kayıtları mühürlü dosyaya basar.
 */
void log_finalize(void);

#endif // LOG_H
