/*
 * COMAR Unified Core - Logging Engine
 * Modernized for 2026 Core Standards
 * * SİSTEM: Günlükleme, Hata Teşhis ve Zaman Damgalama.
 */

#include "config.h"
#include "process.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/**
 * Dosya akışına standart bir zaman damgası basar.
 */
void timestamp(FILE *f) {
    static char buf[128];
    time_t t;
    struct tm *bt;

    time(&t);
    // Güvenli yerel zaman dönüşümü
    bt = localtime(&t);
    if (bt) {
        strftime(buf, sizeof(buf) - 1, "%F %T ", bt);
        fputs(buf, f);
    }
}

/**
 * Süreç kimliğini ve varsa D-Bus gönderen bilgisini mühürler.
 */
void pidstamp(FILE *f) {
    if (my_proc.bus_msg) {
        // D-Bus mesajı varsa gönderen kimliğini ekle
        const char *sender = dbus_message_get_sender(my_proc.bus_msg);
        fprintf(f, "(%d) [bus:%s] ", getpid(), sender ? sender : "unknown");
    } else {
        fprintf(f, "(%d) ", getpid());
    }
}

/**
 * Günlük mesajını hedefe yazdırır.
 */
void log_print(const char *fmt, va_list ap, int error) {
    FILE *f;

    // Çalışma seviyesine göre dosyaya veya standart çıktıya yaz
    if (config_runlevel != 0) {
        f = fopen(config_file_log_traceback, "a");
        if (!f) {
            // Log dosyası açılamazsa stderr'e düş
            f = stderr;
        }
    } else {
        f = stdout;
    }

    timestamp(f);
    pidstamp(f);

    if (error) {
        fprintf(f, "[ERROR] ");
    } else {
        fprintf(f, "[INFO]  ");
    }

    vfprintf(f, fmt, ap);
    
    // Verinin hemen yazılmasını sağla (crash koruması)
    fflush(f);

    if (config_runlevel != 0 && f != stderr) {
        fclose(f);
    }
}

/**
 * Kritik hataları kayıt altına alır.
 */
void log_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(fmt, ap, 1);
    va_end(ap);
}

/**
 * Bilgi mesajlarını kayıt altına alır.
 */
void log_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(fmt, ap, 0);
    va_end(ap);
}

/**
 * Hata ayıklama mesajlarını (debug) kayıt altına alır.
 */
void log_debug(const char *fmt, ...) {
    // Sadece debug modu aktifse günlükle
    if (!config_debug) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log_print(fmt, ap, 0);
    va_end(ap);
}
