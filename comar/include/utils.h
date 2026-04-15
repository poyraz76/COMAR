/*
 * COMAR Utility Helpers (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: C11, Python 3.12+ entegrasyonu.
 * GÜVENLİK: BLAKE3 (Mühürleme), Zstd (Arşivleme).
 * SİSTEM: Systemd-free, Müdür + COMAR uyumlu.
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stddef.h>
#include <stdbool.h>

/* --- Bellek ve Hata Yönetimi --- */

/**
 * @brief Out of Memory (Bellek Yetersiz) hatası durumunda güvenli çıkış yapar.
 */
void oom(void);

/**
 * @brief ZEKA: Teknisyen dostu hata analizi ve çözüm önerisi basar.
 * @param context Hatanın oluştuğu bağlam.
 * @param technical_tip Çözüm için teknik ipucu.
 */
void log_technical_tip(const char *context, const char *technical_tip);

/* --- Dosya ve Dizin İşlemleri --- */

/**
 * @brief Dosyayı belleğe yükler.
 * @return Yüklenen veri. Başarısızlıkta NULL döner.
 */
char *load_file(const char *fname, size_t *sizeptr);

/**
 * @brief Dizin varlığını kontrol eder.
 * @return Dizin varsa 1, yoksa 0.
 */
int check_dir(const char *dir);

/**
 * @brief Eksik dizinleri hiyerarşik olarak oluşturur.
 * @return Başarı durumunda 0.
 */
int create_dir(const char *dir);

/* --- 2026 Mühürleme ve Arşivleme (Yeni) --- */

/**
 * @brief Dosyanın BLAKE3 mühürünü hesaplar.
 * @param fname Dosya yolu.
 * @param out_hash Hash'in yazılacağı tampon (en az 65 byte).
 * @return Başarı durumunda true.
 */
bool blake3_hash_file(const char *fname, char *out_hash);

/**
 * @brief Veriyi Zstd (Level 10-15) ile sıkıştırır.
 * @return Sıkıştırılmış veri boyutu.
 */
size_t zstd_compress_data(const void *src, size_t src_size, void *dst, size_t dst_capacity);

/* --- Metin İşleme Yardımcıları --- */

/**
 * @brief Güvenli substring işlemi yapar.
 */
char *strsub(const char *str, int start, int end);

/**
 * @brief Ayar dosyalarındaki parantez yapılarını (brackets) ayrıştırır.
 */
char *parse_brackets(const char *str, char op, char cl);

#endif /* UTILITIES_H */
