/*
 * COMAR Unified Core - Main Entry Point
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * [span_1](start_span)ALTYAPI: Unified Core 4.0.0 (2026 Edition).[span_1](end_span)
 * GÜVENLİK: Root Enforcement & Sealed Directory Architecture.
 * [span_2](start_span)SİSTEM: Python 3.12+ Integration & D-Bus Main Loop.[span_2](end_span)
 */

#include "config.h"
#include "log.h"
#include "loop.h"
#include "process.h"
#include "script.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * COMAR ana giriş fonksiyonu.
 */
int main(int argc, char *argv[]) {
    // 1. Yapılandırma Motorunu Ateşle
    config_init(argc, argv);

    // 2. Yetki Kontrolü: Sistem servisi mutlaka 'root' olarak başlamalıdır.
    if (getuid() != 0) {
        log_error("Kritik Hata: COMAR çekirdeği 'root' yetkisi olmadan başlatılamaz.\n");
        exit(EXIT_FAILURE);
    }

    // 3. Mevcut Dizinlerin Mühür Kontrolü
    [span_3](start_span)[span_4](start_span)// Veri, Model ve Modül dizinleri kurulum sırasında (CMake) oluşturulmuş olmalıdır.[span_3](end_span)[span_4](end_span)
    if (check_dir(config_dir_data) != 0 || 
        check_dir(config_dir_models) != 0 || 
        check_dir(config_dir_modules) != 0) {
        log_error("Hata: Temel sistem dizinleri eksik veya mühürsüz. Kurulumu kontrol edin.\n");
        exit(EXIT_FAILURE);
    }

    // 4. Çalışma Zamanı Dizinlerini Mühürle
    [span_5](start_span)// Betikler, Uygulama kayıtları ve Loglar için gerekli yollar oluşturulur.[span_5](end_span)
    if (create_dir(config_dir_scripts) != 0 || 
        create_dir(config_dir_apps) != 0 || 
        create_dir(config_dir_log) != 0) {
        log_error("Hata: Çalışma zamanı dizinleri (scripts/apps/log) oluşturulamadı.\n");
        exit(EXIT_FAILURE);
    }

    // 5. Günlükleme Seviyesi Ayarı
    // --print parametresi yoksa günlükler dosyaya mühürlenir.
    if (config_print == 0) {
        config_runlevel = 1;
    }

    log_info("COMAR Unified Core 4.0 Başlatılıyor...\n");
    log_debug("Sistem Dizinleri Mühürlendi:\n");
    log_debug("  Modüller : %s\n", config_dir_modules);
    log_debug("  Modeller : %s\n", config_dir_models);
    log_debug("  Betikler : %s\n", config_dir_scripts);
    log_debug("  Günlük   : %s\n\n", config_file_log_access);

    // 6. Python VM Motorunu 2026 Standartlarında Başlat
    // Python 3.12+ entegrasyonu ve mühürlü istisnalar burada yüklenir.
    if (script_init() != 0) {
        log_error("Kritik Hata: Python VM başlatılamadı.\n");
        exit(EXIT_FAILURE);
    }

    // 7. Süreç Yönetim Motorunu (Process Manager) Hazırla
    // Alt süreçlerin takibi ve sinyal yakalama mekanizması mühürlenir.
    proc_init();

    // 8. Ana Döngüye Gir (The Big Loop)
    [span_6](start_span)// D-Bus üzerindeki tüm asenkron talepler buradan yönetilir.[span_6](end_span)
    log_info("Çekirdek D-Bus üzerinde dinlemede (Unified Loop).\n");
    loop_exec();

    // 9. Temizlik ve Kapanış
    log_info("Sistem kapatılıyor, Python VM mühürleri sökülüyor.\n");
    script_finalize();

    return 0;
}
