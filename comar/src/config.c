/*
 * COMAR Unified Core - Configuration & CLI Engine
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * ALTYAPI: Unified Core 4.0.0 (2026 Edition).
 * SİSTEM: Dinamik Dizin İnşası ve Güvenli Argüman Ayrıştırma.
 */

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "config.h"
#include "utils.h"

// D-Bus ve Sistem Tanımlamaları
char *config_server_address = DBUS_SERVER_ADDRESS;
const char *config_unique_address;
char *config_service_name = DBUS_SERVICE_NAME;
char *config_interface = DBUS_INTERFACE_PREFIX;
int config_timeout = IDLE_TIMEOUT;

// Dizin Yapılandırması (FHS 2026 Standartları)
char *config_dir_data = DATA_DIR; // Örn: /var/lib/comar
char *config_dir_models;
char *config_dir_modules;
char *config_dir_scripts;
char *config_dir_apps;
char *config_dir_log = LOG_DIR; // Örn: /var/log/comar

// Günlük ve Süreç Dosyaları
char *config_file_log_access;
char *config_file_log_traceback;
char *config_file_pid = PID_FILE;

// Çalışma Modları
int config_debug = 0;
int config_print = 0;
int config_runlevel = 0;
int config_ignore_missing = 0;

// Komut Satırı Seçenekleri (getopt_long)
static struct option longopts[] = {
    { "busname",        required_argument, NULL, 'b' },
    { "datadir",        required_argument, NULL, 'd' },
    { "debug",          0,                 NULL, 'g' },
    { "ignore-missing", 0,                 NULL, 'i' },
    { "logdir",         required_argument, NULL, 'l' },
    { "print",          0,                 NULL, 'p' },
    { "socket",         required_argument, NULL, 's' },
    { "timeout",        required_argument, NULL, 't' },
    { "help",           0,                 NULL, 'h' },
    { "version",        0,                 NULL, 'v' },
    { NULL,             0,                 NULL, 0   }
};

static char *shortopts = "b:d:gil:ps:t:hv";

/**
 * Kullanım kılavuzunu ekrana basar.
 */
static void print_usage(const char *name) {
    printf(
        "Kullanım: %s [SEÇENEKLER]\n\n"
        "  -b, --busname  [İSİM]  D-Bus servis ismi (Varsayılan: %s)\n"
        "  -d, --datadir  [DİZİN] Veri depolama dizini (Varsayılan: %s)\n"
        "  -g, --debug            Hata ayıklama modunu etkinleştir.\n"
        "  -i, --ignore-missing   Eksik fonksiyon hatalarını yoksay.\n"
        "  -l, --logdir   [DİZİN] Günlük depolama dizini (Varsayılan: %s)\n"
        "  -p, --print            Konsola yazdır ve zaman aşımını devre dışı bırak.\n"
        "  -s, --socket   [SOKET] D-Bus soket adresi (Varsayılan: %s)\n"
        "  -t, --timeout  [SN]    Boşta kalma zaman aşımı (Varsayılan: %d sn, 0: kapalı)\n"
        "  -h, --help             Bu yardım metnini bas ve çık.\n"
        "  -v, --version          Sürüm bilgisini bas ve çık.\n\n"
        "Hataları %s adresine bildirin.\n",
        name, config_service_name, config_dir_data, config_dir_log,
        config_server_address, config_timeout, WWW_BUGS
    );
}

/**
 * Sürüm bilgisini mühürlü olarak ekrana basar.
 */
static void print_version(void) {
    printf("COMAR Unified Core %s\n", VERSION);
}

/**
 * Yapılandırma motorunu başlatır ve argümanları ayrıştırır.
 */
void config_init(int argc, char *argv[]) {
    int c, i;

    // Argüman ayrıştırma döngüsü
    while ((c = getopt_long(argc, argv, shortopts, longopts, &i)) != -1) {
        switch (c) {
            case 'b': config_service_name = strdup(optarg); break;
            case 'd': config_dir_data = strdup(optarg); break;
            case 'g': config_debug = 1; break;
            case 'i': config_ignore_missing = 1; break;
            case 'l': config_dir_log = strdup(optarg); break;
            case 'p': config_print = 1; config_timeout = 0; break;
            case 's': config_server_address = strdup(optarg); break;
            case 't': config_timeout = (int)strtol(optarg, NULL, 0); break;
            case 'h': print_usage(argv[0]); exit(0);
            case 'v': print_version(); exit(0);
            default:  exit(1);
        }
        if (optarg && !config_service_name && c != 't') oom(); // Bellek kontrolü
    }

    // Dinamik Yol İnşası (Path Construction)
    // Bellek ihtiyacı: strlen(base) + '/' + strlen(sub) + '\0'
    
    auto void build_path(char **target, const char *base, const char *sub) {
        int size = (int)(strlen(base) + strlen(sub) + 2);
        *target = malloc(size);
        if (!*target) oom();
        snprintf(*target, size, "%s/%s", base, sub);
    }

    build_path(&config_dir_modules, config_dir_data, "modules");
    build_path(&config_dir_models,  config_dir_data, "models");
    build_path(&config_dir_scripts, config_dir_data, "scripts");
    build_path(&config_dir_apps,    config_dir_data, "apps");
    build_path(&config_file_log_access,    config_dir_log, "access.log");
    build_path(&config_file_log_traceback, config_dir_log, "trace.log");
}
