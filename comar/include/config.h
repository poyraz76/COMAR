/*
 * COMAR System Configuration (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: Python 3.12+, x86_64, SQLite DB.
 * GÜVENLİK: BLAKE3 Mühürleme ve SHA-512 Doğrulama.
 * SİSTEM: Systemd-free (Müdür + COMAR), TOML Manifest.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* --- 2026 Sistem Sürümleri --- */
#ifndef VERSION
#define VERSION "4.0.0"
#endif

/* --- D-Bus Haberleşme Protokolü --- */
#ifndef DBUS_SERVER_ADDRESS
#define DBUS_SERVER_ADDRESS "unix:path=/run/dbus/system_bus_socket"
#endif

#ifndef DBUS_SERVICE_NAME
#define DBUS_SERVICE_NAME "tr.org.pardus.comar"
#endif

#ifndef DBUS_INTERFACE_PREFIX
#define DBUS_INTERFACE_PREFIX "tr.org.pardus.comar"
#endif

/* --- Zaman Aşımı ve Performans (Asyncio Uyumlu) --- */
#ifndef IDLE_TIMEOUT
#define IDLE_TIMEOUT 120  /* 2026 Standardı: 120 saniye */
#endif

/* --- Merkezi Dizin ve Dosya Yolları --- */
#ifndef DIR_DATA
#define DIR_DATA "/var/lib/comar"
#endif

#ifndef DB_PATH
#define DB_PATH "/var/lib/pisi/inventory.db"
#endif

#ifndef DIR_LOG
#define DIR_LOG "/var/log/comar"
#endif

#ifndef FILE_PID
#define FILE_PID "/run/comar.pid"
#endif

/* --- Global Yapılandırma Değişkenleri --- */
extern char *config_server_address;
extern const char *config_unique_address;
extern char *config_service_name;
extern char *config_interface;

/* Merkezi Envanter ve Dizinler */
extern char *config_db_path;     /* SQLite envanter yolu */
extern char *config_dir_data;
extern char *config_dir_log;
extern char *config_file_pid;

/* Çalışma Bayrakları (Flags) */
extern int config_timeout;
extern bool config_debug;
extern bool config_print;
extern bool config_use_blake3;   /* 2026: BLAKE3 mühürleme kontrolü */
extern bool config_use_zstd;     /* 2026: Zstd arşivleme kontrolü */
extern bool config_zeka_enabled; /* 2026: AI Hata Analizi (
