/*
 * COMAR Process Management Layer (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: C11, Asyncio Sinyal Yönetimi entegrasyonu.
 * GÜVENLİK: Mühürlü Süreç İcrası (Sealed Execution).
 * SİSTEM: Systemd-free, Müdür + COMAR uyumlu süreç takibi.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <Python.h>
#include <dbus/dbus.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

/**
 * @brief Alt süreç (child) veri yapısı.
 * 2026 Protokolü: Süreçlerin başlangıç zamanı ve mühür durumu takip edilir.
 */
struct ProcChild {
    int from;               /* Okuma kanalı (pipe) */
    pid_t pid;              /* Süreç kimliği */
    DBusMessage *bus_msg;   /* Süreci tetikleyen D-Bus mesajı */
    time_t start_time;      /* Otonom zaman aşımı kontrolü için */
    bool is_sealed;         /* Sürecin mühürlü bir binary/script olduğu doğrulaması */
};

/**
 * @brief Merkezi süreç yönetim yapısı.
 */
struct Proc {
    struct ProcChild parent;    /* Ana süreç bilgisi */
    DBusMessage *bus_msg;       /* Global mesaj havuzu */
    int nr_children;            /* Aktif alt süreç sayısı */
    int max_children;           /* Maksimum izin verilen süreç sınırı */
    struct ProcChild *children; /* Alt süreç envanteri */
};

/* Global süreç yöneticisi */
extern struct Proc my_proc;

/* --- 2026 Protokol Fonksiyonları --- */

/**
 * @brief Süreç yöneticisini ve sinyal yakalayıcıları (SIGCHLD) başlatır.
 * @return Başarıda 0.
 */
int proc_init(void);

/**
 * @brief Bir betiği veya komutu alt süreç olarak çatallar (fork).
 * 2026 Kuralı: İcra öncesi mühür kontrolü (script_verify_seal) zorunludur.
 * * @param child_func Alt süreçte çalışacak fonksiyon.
 * @param msg Tetikleyici D-Bus mesajı.
 * @return Oluşturulan alt süreç yapısı veya hata durumunda NULL.
 */
struct ProcChild *proc_fork(void (*child_func)(DBusMessage *msg), DBusMessage *msg);

/**
 * @brief Biten veya zaman aşımına uğrayan alt süreci envanterden temizler.
 */
void rem_child(int nr);

/**
 * @brief Zombi süreçleri (zombie processes) otonom olarak temizler.
 * Bu fonksiyon Asyncio döngüsüyle paralel çalışacak şekilde tasarlanmıştır.
 */
void proc_reap_children(void);

/**
 * @brief ZEKA: Süreç hatalarında (Fork failure, OOM) teknisyen analizi raporlar.
 */
void report_process_error(const char *context, int error_code);

#endif /* PROCESS_H */
