/*
 * COMAR Unified Core Utilities
 * Copyright (c) 2026
 *
 * ALTYAPI: Modern C99/C11 Standartları
 * GÜVENLİK: TOCTOU Korumalı Dosya İşlemleri ve Güvenli Substring
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

#include "log.h"

/**
 * Bellek yetersizliği durumunda sistemi güvenli bir şekilde durdurur.
 */
void oom() {
    log_error("Kritik Hata: Bellek yetersiz (Out of memory). Çekirdek durduruluyor.\n");
    exit(EXIT_FAILURE);
}

/**
 * Bir dosyanın içeriğini güvenli bir şekilde belleğe yükler.
 * TOCTOU (Time-of-Check to Time-of-Use) saldırılarına karşı fstat kullanılır.
 */
char *load_file(const char *fname, int *sizeptr) {
    FILE *f;
    struct stat fs;
    size_t size;
    char *data;

    f = fopen(fname, "rb");
    if (!f) {
        log_error("Dosya açılamadı: '%s' (Hata: %s)\n", fname, strerror(errno));
        return NULL;
    }

    // Dosya boyutu, dosya açıldıktan sonra descriptor üzerinden alınır.
    if (fstat(fileno(f), &fs) != 0) {
        fclose(f);
        return NULL;
    }

    size = (size_t)fs.st_size;
    if (sizeptr) *sizeptr = (int)size;

    data = malloc(size + 1);
    if (!data) {
        fclose(f);
        oom();
    }

    if (fread(data, 1, size, f) < size && ferror(f)) {
        log_error("Dosya okunurken hata oluştu: '%s'\n", fname);
        free(data);
        fclose(f);
        return NULL;
    }

    data[size] = '\0'; // Güvenli sonlandırma.
    fclose(f);

    return data;
}

/**
 * Dizinin varlığını ve erişim yetkilerini kontrol eder.
 */
int check_dir(char *dir) {
    struct stat fs;

    if (stat(dir, &fs) != 0) {
        log_error("Eksik dizin: '%s'\n", dir);
        return -1;
    }

    if (!S_ISDIR(fs.st_mode)) {
        log_error("Yol bir dizin değil: '%s'\n", dir);
        return -1;
    }

    if (access(dir, R_OK | W_OK) != 0) {
        log_error("Dizin erişim yetkisi yetersiz (R/W): '%s'\n", dir);
        return -1;
    }

    return 0;
}

/**
 * Dizin yoksa oluşturur, varsa yazma yetkisini kontrol eder.
 */
int create_dir(char *dir) {
    struct stat fs;

    if (stat(dir, &fs) != 0) {
        // Kullanıcı yetkileriyle dizin oluşturma (rwxr-x---)
        if (mkdir(dir, 0750) != 0) {
            log_error("Dizin oluşturulamadı: '%s' (Hata: %s)\n", dir, strerror(errno));
            return -1;
        }
    } else if (access(dir, W_OK) != 0) {
        log_error("Dizine yazma yetkisi yok: '%s'\n", dir);
        return -1;
    }

    return 0;
}

/**
 * Bir katarın (string) belirli bir bölümünü güvenli bir şekilde döndürür.
 * Sınır kontrolleri eklenerek bellek taşmaları engellenmiştir.
 */
char *strsub(const char *str, int start, int end) {
    if (!str) return NULL;

    size_t len = strlen(str);
    
    // Sınır düzeltmeleri
    if (start < 0) start = 0;
    if (end <= 0 || (size_t)end > len) end = (int)len;
    if (start >= end) return strdup("");

    size_t sub_len = (size_t)(end - start);
    char *new_src = malloc(sub_len + 1);
    
    if (!new_src) oom();

    memcpy(new_src, str + start, sub_len);
    new_src[sub_len] = '\0';

    return new_src;
}
