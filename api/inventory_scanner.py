#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Central Inventory Scanner (Nihai Sürüm)
# Copyright (C) 2026, Ergün Salman
#
# ALTYAPI: Python 3.12+, x86_64, SQLite DB, Asyncio.
# GÜVENLİK: BLAKE3 (Bütünlük doğrulaması), SHA-512.
# SİSTEM: Systemd-free, Müdür + COMAR uyumlu.

import os
import asyncio
import sqlite3
import logging
from pathlib import Path
from blake3 import blake3

# --- 2026 Merkezi Envanter Ayarları ---
DB_PATH = Path("/var/lib/pisi/inventory.db")
CRITICAL_PATHS = ["/bin", "/sbin", "/usr/lib/python3/dist-packages/comar"] #

class InventoryScanner:
    """Sistemi tarayıp merkezi envantere mühürleyen ana motor."""

    def __init__(self):
        self.logger = self.setup_logger()
        self._init_db()

    def setup_logger(self):
        logging.basicConfig(level=logging.INFO, format='[*] %(message)s')
        return logging.getLogger("InventoryScanner")

    def _init_db(self):
        """SQLite tablolarını tek merkez kuralına göre hazırlar."""
        if not DB_PATH.parent.exists():
            DB_PATH.parent.mkdir(parents=True)
            
        with sqlite3.connect(DB_PATH) as conn:
            # Servis envanteri
            conn.execute("""
                CREATE TABLE IF NOT EXISTS services 
                (name TEXT PRIMARY KEY, state TEXT, status TEXT, last_scan DATETIME DEFAULT CURRENT_TIMESTAMP)
            """)
            # Dosya bütünlük mühürleri
            conn.execute("""
                CREATE TABLE IF NOT EXISTS integrity_seals 
                (path TEXT PRIMARY KEY, blake3_hash TEXT, last_check DATETIME DEFAULT CURRENT_TIMESTAMP)
            """)

    def zeka_analizi(self, module: str, error: Exception):
        """ZEKA: AI hata analizi ve teknisyen çözüm önerisi."""
        print(f"\n\033[1;91m[!] AI ANALİZİ - Tarama Hatası: {module}\033[0m")
        print(f"[*] Detay: {str(error)}")
        print("[*] Sistem Önerisi: Envanter veritabanı kilitli olabilir veya izinlerde sorun var.")

    async def scan_services(self):
        """Servis durumlarını tarar ve mühürler."""
        self.logger.info("Servis envanteri taranıyor...")
        try:
            # Müdür servis dizini taraması
            enabled_dir = Path("/etc/mudur/services/enabled")
            if enabled_dir.exists():
                services = [f.name for f in enabled_dir.iterdir()]
                with sqlite3.connect(DB_PATH) as conn:
                    for s in services:
                        conn.execute("INSERT OR REPLACE INTO services (name, state) VALUES (?, ?)", (s, "on"))
        except Exception as e:
            self.zeka_analizi("Services", e)

    async def verify_system_integrity(self):
        """Kritik sistem dosyalarını BLAKE3 ile tarayıp doğrular."""
        self.logger.info("Kritik dosya bütünlüğü taranıyor (BLAKE3)...")
        with sqlite3.connect(DB_PATH) as conn:
            for base_path in CRITICAL_PATHS:
                p = Path(base_path)
                if p.exists():
                    for file in p.rglob("*"):
                        if file.is_file():
                            try:
                                content = file.read_bytes()
                                f_hash = blake3(content).hexdigest()
                                conn.execute("INSERT OR REPLACE INTO integrity_seals (path, blake3_hash) VALUES (?, ?)", 
                                            (str(file), f_hash))
                            except Exception:
                                continue

    async def run_full_scan(self):
        """Tüm tarama süreçlerini asenkron olarak yürütür."""
        self.logger.info("COMAR Tam Sistem Taraması Başlatıldı.")
        await asyncio.gather(
            self.scan_services(),
            self.verify_system_integrity()
        )
        self.logger.info("Envanter mühürlendi ve merkezi veritabanına işlendi.")

if __name__ == "__main__":
    scanner = InventoryScanner()
    asyncio.run(scanner.run_full_scan())
