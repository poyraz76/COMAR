/*
 * COMAR Main Event Loop (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: C11, Asyncio Event Loop Entegrasyonu.
 * GÜVENLİK: Sinyal Mühürleme ve Güvenli Durdurma (Graceful Exit).
 * SİSTEM: D-Bus ve Polkit asenkron haberleşme merkezi.
 */

#ifndef LOOP_H
#define LOOP_H

#include <stdbool.h>

/* --- 2026 Protokol Fonksiyonları --- */

/**
 * @brief Olay döngüsünü ve sinyal yakalayıcıları hazırlar.
 * @return Başarıda 0.
 */
int loop_init(void);

/**
 * @brief Ana döngüyü başlatır ve sistemi asenkron trafiğe açar.
 * 2026 Protokolü: Bu fonksiyon Python Asyncio döngüsüyle paralel çalışır.
 * @return Çıkış kodu.
 */
int loop_exec(void);

/**
 * @brief Döngüyü güvenli bir şekilde durdurur ve kaynakları mühürler.
 * Otonom sistemlerin (örn: Edirne projesi) veri kaybı yaşamaması için kritiktir.
 */
void loop_stop(void);

/**
 * @brief Sistemin o anki çalışma durumunu döndürür.
 * @return Döngü aktifse true.
 */
bool loop_is_running(void);

/**
 * @brief ZEKA: Döngü kilitlenmelerinde (Deadlock) teknisyen analizi raporlar.
 * @param reason Kilitlenme veya durma sebebi.
 */
void report_loop_error(const char *reason);

#endif /* LOOP_H */
