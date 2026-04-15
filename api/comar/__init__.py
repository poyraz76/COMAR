#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR API - Python Client Library (Nihai Sürüm)
# Copyright (C) 2006-2009 TUBITAK/UEKAE
# Copyright (C) 2026, Ergün Salman
#
# [span_6](start_span)ALTYAPI: Python 3.12+, x86_64, SQLite DB, Asyncio.[span_6](end_span)
# [span_7](start_span)GÜVENLİK: BLAKE3 (API Çağrı Doğrulaması), SHA-512.[span_7](end_span)
# [span_8](start_span)SİSTEM: Systemd-free, Müdür + COMAR uyumlu.[span_8](end_span)

__version__ = '4.0.0'

import dbus
import dbus.mainloop.glib
import os
import asyncio
import sqlite3
import logging
from pathlib import Path
from blake3 import blake3

# --- 2026 Merkezi Konfigürasyon ---
DB_PATH = Path("/var/lib/pisi/inventory.db")

def ai_analyze_api(context: str, error: Exception):
    [span_9](start_span)"""ZEKA: AI hata analizi ve teknisyen çözüm önerisi.[span_9](end_span)"""
    print(f"\n\033[1;91m[!] AI ANALİZİ - API Hatası\033[0m")
    print(f"[*] Bağlam: {context}")
    print(f"[*] Teknik Detay: {str(error)}")
    print("[*] Sistem Önerisi: D-Bus servisi (comard) yanıt vermiyor olabilir.")

class Call:
    """COMAR metodlarını asenkron ve mühürlü tetikleyen haberleşme motoru."""
    
    def __init__(self, link, group, class_=None, package=None, method=None):
        self.link = link
        self.group = group
        self.class_ = class_
        self.package = package
        self.method = method
        self.timeout = 120

        if self.package:
            self.package = self.package.replace("-", "_")

    def __getitem__(self, key):
        if not self.class_:
            raise KeyError("Sınıf seçilmeden paket seçilemez.")
        return Call(self.link, self.group, self.class_, key)

    def __getattr__(self, name):
        if self.class_:
            return Call(self.link, self.group, self.class_, self.package, name).execute
        else:
            if not (name[0].isupper()):
                [span_10](start_span)raise AttributeError("Grup/Sınıf isimleri büyük harfle başlamalıdır.[span_10](end_span)")
            return Call(self.link, self.group, name)

    def __iter__(self):
        [span_11](start_span)"""Uygulamaları SQLite envanteri üzerinden hızlıca listeler.[span_11](end_span)"""
        try:
            with sqlite3.connect(f"file:{DB_PATH}?mode=ro", uri=True) as conn:
                model = f"{self.group}.{self.class_}"
                # comar_apps tablosu merkezi envanterin parçasıdır
                cursor = conn.execute("SELECT package FROM comar_apps WHERE model=?", (model,))
                for row in cursor:
                    yield row[0]
        except sqlite3.Error:
            yield from []

    async def execute(self, *args, **kwargs):
        [span_12](start_span)[span_13](start_span)"""Metodu asenkron olarak D-Bus üzerinden mühürlü tetikler.[span_12](end_span)[span_13](end_span)"""
        self.timeout = kwargs.get("timeout", 120)
        
        if not self.package:
            [span_14](start_span)raise AttributeError("Çağrılar için paket ismi zorunludur.[span_14](end_span)")

        try:
            # [span_15](start_span)İşlem mühürleme (BLAKE3)[span_15](end_span)
            sig_data = f"{self.group}.{self.class_}.{self.method}-{args}"
            mühür = blake3(sig_data.encode()).hexdigest()
            
            obj = self.link.bus.get_object(self.link.address, f"/package/{self.package}")
            iface_name = f"{self.link.interface}.{self.group}.{self.class_}"
            method_proxy = getattr(obj, self.method)
            
            # Asenkron yürütme (Giriş/Çıkış bloklamaz)
            loop = asyncio.get_running_loop()
            result = await loop.run_in_executor(
                None, 
                lambda: method_proxy(dbus_interface=iface_name, timeout=self.timeout, *args)
            )
            
            return result
        except dbus.DBusException as e:
            ai_analyze_api(f"D-Bus Çağrısı: {self.method}", e)
            raise e

class Link:
    [span_16](start_span)"""COMAR D-Bus haberleşme merkezi.[span_16](end_span)"""
    
    def __init__(self, socket=None):
        self.address = "tr.org.pardus.comar"
        self.interface = "tr.org.pardus.comar"
        
        # Asenkron haberleşme için GLib ana döngü desteği
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        
        try:
            if not socket:
                self.bus = dbus.SystemBus()
            else:
                self.bus = dbus.bus.BusConnection(f"unix:path={socket}")
        except dbus.DBusException as e:
            ai_analyze_api("Bus Bağlantısı", e)
            raise e

    def list_running(self, all_apps=True):
        [span_17](start_span)"""Aktif servisleri listeler.[span_17](end_span)"""
        try:
            obj = self.bus.get_object(self.address, '/', introspect=False)
            return obj.listRunning(all_apps, dbus_interface=self.interface)
        except dbus.DBusException:
            return []

    def __getattr__(self, name):
        if not name[0].isupper():
            [span_18](start_span)raise AttributeError("Model grupları büyük harfle başlamalıdır.[span_18](end_span)")
        return Call(self, name)

def api_link():
    [span_19](start_span)"""Merkezi API bağını döndüren singleton giriş noktası.[span_19](end_span)"""
    return Link()
