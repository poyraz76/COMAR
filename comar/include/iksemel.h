/* ** COMAR / Müdür XML-TOML Unified Parser (Nihai Protokol)
** Copyright (C) 2005-2010 TUBITAK/UEKAE
** Copyright (C) 2026, Ergün Salman
**
** ALTYAPI: Python 3.12+, x86_64, SQLite Entegrasyonu.
** GÜVENLİK: BLAKE3 (Mühürleme), MTREE Manifest Doğrulama.
** ARŞİVLEME: Zstd (Ultra Sıkıştırma), TOML Çıktı Desteği.
*/

#ifndef IKSEMEL_H
#define IKSEMEL_H 1

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. MODERN: Şema ve Bütünlük Denetimi --- */

/**
 * Dosyayı BLAKE3 mühürünü doğrulayarak yükler.
 * Mühürsüz veya ihlalli dosyalar sisteme sızamaz.
 */
int iks_load_sealed(const char *fname, struct iks_struct **xptr);

/**
 * MTREE manifest yapısını kullanarak dosya sistemini doğrular.
 */
bool iks_verify_mtree(const char *manifest_path);

/* --- 2. MODERN: XML'den TOML'a Dönüşüm Motoru --- */

/**
 * pspec.xml ve actions.py girdilerini alarak package.toml üretir.
 * Bu, COMAR 2.0 mimarisinin temel kuralıdır.
 */
char *iks_to_toml(const char *xml_data, size_t len);

/**
 * Değişen kısımları Zstd sıkıştırmasıyla yamalar (Differential Patch).
 */
int iks_apply_patch_zstd(const char *target_file, const char *zstd_diff);

/* --- 3. MODERN: ZEKA (AI Hata Analizi) Kancaları --- */

/**
 * Ayrıştırma hatası durumunda AI motoruna (ZEKA) detaylı bağlam sunar.
 * Hatanın nedenini ve çözüm önerisini (Poyraz76 standartları) raporlar.
 */
void iks_ai_diagnose(const char *context, int error_code);

/* --- 4. ASENKRON VE AKIŞ YÖNETİMİ --- */

/**
 * Büyük paket listelerini Asyncio döngüsüyle uyumlu, bloklamayan yapıda ayrıştırır.
 */
int iks_parse_async(const char *stream_data, size_t len, void *user_data);

/***** Çekirdek Yapı (Modernize Edilmiş) *****/

/* Yığın (Stack) Yönetimi - İzole Sandbox uyumlu */
struct ikstack_struct;
typedef struct ikstack_struct ikstack;
ikstack *iks_stack_new (size_t meta_chunk, size_t data_chunk);
void iks_stack_delete (ikstack *s);

/* DOM ve SAX Düğümleri */
enum ikstype {
    IKS_NONE = 0,
    IKS_TAG,
    IKS_ATTRIBUTE,
    IKS_CDATA,
    IKS_TOML_KEY /* TOML anahtar desteği */
};

typedef struct iks_struct iks;
iks *iks_new (const char *name);
void iks_delete (iks *x);
iks *iks_insert (iks *x, const char *name);
char *iks_find_attrib (iks *x, const char *name);

#ifdef __cplusplus
}
#endif

#endif
