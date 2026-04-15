/*
 * COMAR Authorization & Policy Layer (Unified Core Edition)
 * Copyright (c) 2005-2010, TUBITAK/UEKAE
 * Copyright (c) 2026, Ergün Salman
 *
 * ALTYAPI: Polkit 0.96+ entegrasyonu.
 * GÜVENLİK: BLAKE3 Politika Mühürleme (Policy Sealing).
 * STANDART: COMAR 2.0 Action ID hiyerarşisi.
 */

#ifndef POLICY_H
#define POLICY_H

#include <Python.h>
#include <dbus/dbus.h>
#include <stdbool.h>

/* --- Polkit Yetki Sonuçları --- */
#define POLICY_YES   1  /* Yetki verildi. */
#define POLICY_AUTH  0  /* Kimlik doğrulaması gerekiyor. */
#define POLICY_NO   -1  /* Yetki reddedildi. */

/* --- 2026 Protokol Fonksiyonları --- */

/**
 * @brief Politika yöneticisini ve Polkit bağlantısını başlatır.
 * @return Başarıda 0.
 */
int policy_init(void);

/**
 * @brief Polkit kaynaklarını temizler.
 */
void policy_finalize(void);

/**
 * @brief Bir eylemin (action) yetki durumunu kontrol eder.
 * 2026 Protokolü: tr.org.pardus.comar.* action ID'leri kullanılır.
 * * @param sender D-Bus üzerindeki gönderici (Unique Name).
 * @param action Polkit action ID (örn: tr.org.pardus.comar.netstack.setip).
 * @param result POLICY_* sonuçlarından biri.
 * @return Başarıda 0, iletişim hatasında -1.
 */
int policy_check(const char *sender, const char *action, int *result);

/**
 * @brief Politika dosyasının (.policy) BLAKE3 mühürünü envanterden doğrular.
 * Bu sayede sistem dışı birinin yetki kurallarını değiştirmesi engellenir.
 * * @param action_id Kontrol edilecek eylem kimliği.
 * @return Mühür geçerliyse true.
 */
bool policy_verify_seal(const char *action_id);

/**
 * @brief ZEKA: Yetki reddedildiğinde teknisyen analizi ve öneri sunar.
 * @param action Reddedilen eylem.
 * @param sender Talebi gönderen süreç.
 */
void report_policy_violation(const char *action, const char *sender);

#endif /* POLICY_H */
