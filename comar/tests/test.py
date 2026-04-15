#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Unified Core - D-Bus Testing Script (2026 Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# ALTYAPI: Python 3.12+, Unified D-Bus Core.
# AMAÇ: Çekirdek metotlarını ve Model çağrılarını test eder.

import dbus
import sys

# [span_3](start_span)[span_4](start_span)2026 Unified Core Adreslemeleri[span_3](end_span)[span_4](end_span)
COMAR_BUS = 'tr.org.pardus.comar'
APACHE_PATH = '/package/apache'
SERVICE_MODEL = f'{COMAR_BUS}.System.Service'

try:
    # Sistem veri yoluna bağlan
    bus = dbus.SystemBus()

    # 1. Aşama: Dil Ayarı (Locale) Mühürleme
    # Root nesnesi üzerinden ana çekirdek metodunu çağırıyoruz.
    core_obj = bus.get_object(COMAR_BUS, '/', introspect=False)
    core_obj.setLocale('tr', dbus_interface=COMAR_BUS)
    print("[✓] Sistem dili 'tr' olarak mühürlendi.")

    # 2. Aşama: Model Metodu Çağrısı (System.Service)
    # Apache paketinin durum bilgisini mühürlü model üzerinden alıyoruz.
    package_obj = bus.get_object(COMAR_BUS, APACHE_PATH, introspect=False)
    
    # info() metodu System.Service modeli içinde tanımlıdır.
    info_result = package_obj.info(dbus_interface=SERVICE_MODEL, timeout=60)
    
    print(f"\n[i] Apache Servis Bilgisi:\n{info_result}")

except dbus.exceptions.DBusException as e:
    print(f"[!] D-Bus Hatası: {e.get_dbus_message()}")
    if "Comar.PolicyKit" in str(e):
        print("[-] Yetki Reddedildi: Polkit mühür doğrulaması başarısız.")
except Exception as e:
    print(f"[!] Beklenmeyen Hata: {e}")
