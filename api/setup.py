#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR API & Inventory Master Setup Engine
# Copyright (C) 2026, Ergün Salman
#
# ALTYAPI: Python 3.12+, x86_64, SQLite DB.
# GÜVENLİK: BLAKE3 (Bütünlük doğrulaması ve mühürleme).
# SİSTEM: Systemd-free (Müdür + COMAR uyumlu).

import os
import sys
import glob
import shutil
import sqlite3
import tarfile
import zstandard as zstd
from pathlib import Path
from blake3 import blake3

# --- 2026 Standartları ---
VERSION = "4.0.0"
DB_PATH = Path("/var/lib/pisi/inventory.db")
# Tüm modüller ve araçlar dahil edildi
DIST_FILES = [
    "setup.py",
    "comar/*.py",
    "inventory_scanner.py"
]

class ComarSetup:
    """Çomar API ve Envanter bileşenlerini sisteme mühürleyen kurulum motoru."""

    def __init__(self, prefix="/"):
        self.prefix = Path(prefix)
        self.logger = self.setup_logger()

    def setup_logger(self):
        import logging
        logging.basicConfig(level=logging.INFO, format='[*] %(message)s')
        return logging.getLogger("ComarSetup")

    def get_blake3(self, path: Path) -> str:
        """Dosyanın BLAKE3 mühürünü hesaplar."""
        return blake3(path.read_bytes()).hexdigest()

    def update_inventory(self, source: str, dest: str):
        """Kurulan dosyayı SQLite envanterine mühürler."""
        try:
            if not (self.prefix / DB_PATH.relative_to('/')).parent.exists():
                (self.prefix / DB_PATH.relative_to('/')).parent.mkdir(parents=True, exist_ok=True)
            
            # Prefix desteğiyle DB bağlantısı
            actual_db = self.prefix / DB_PATH.relative_to('/')
            with sqlite3.connect(actual_db) as conn:
                conn.execute("""
                    CREATE TABLE IF NOT EXISTS file_inventory 
                    (path TEXT PRIMARY KEY, source TEXT, blake3_hash TEXT, version TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)
                """)
                f_hash = self.get_blake3(Path(source))
                conn.execute("INSERT OR REPLACE INTO file_inventory (path, source, blake3_hash, version) VALUES (?, ?, ?, ?)",
                            (str(dest), source, f_hash, VERSION))
        except Exception as e:
            self.logger.error(f"Envanter mühürleme hatası: {e}")

    def install(self):
        """API dosyalarını ve tarayıcıyı hiyerarşik düzende sisteme kurar."""
        self.logger.info(f"COMAR {VERSION} Merkezi Kurulumu Başlatıldı...")
        
        # 1. Kütüphane Kurulumu (/usr/lib/python3/dist-packages/comar)
        lib_path = self.prefix / "usr/lib/python3/dist-packages/comar"
        lib_path.mkdir(parents=True, exist_ok=True)

        for f in glob.glob("comar/*.py"):
            source = Path(f)
            dest = lib_path / source.name
            shutil.copy(source, dest)
            # Sistem içi mutlak yolu kaydediyoruz
            sys_dest = Path("/") / "usr/lib/python3/dist-packages/comar" / source.name
            self.update_inventory(str(source), str(sys_dest))
            self.logger.info(f"[+] Mühürlendi: {source.name}")

        # 2. Merkezi Tarayıcı Kurulumu (/usr/sbin/comar-scanner)
        sbin_path = self.prefix / "usr/sbin"
        sbin_path.mkdir(parents=True, exist_ok=True)
        scanner_source = Path("inventory_scanner.py")
        scanner_dest = sbin_path / "comar-scanner"

        if scanner_source.exists():
            shutil.copy(scanner_source, scanner_dest)
            os.chmod(scanner_dest, 0o755) # Çalıştırma yetkisi
            sys_scanner_dest = Path("/") / "usr/sbin/comar-scanner"
            self.update_inventory(str(scanner_source), str(sys_scanner_dest))
            self.logger.info(f"[+] Sistem Karargahı kuruldu: {scanner_dest}")

        self.logger.info("COMAR kurulumu ve envanter mühürlemesi başarıyla tamamlandı.")

if __name__ == "__main__":
    args = sys.argv[1:]
    if not args:
        print("Kullanım: setup.py [install|dist] [prefix]")
        sys.exit(1)

    command = args[0]
    prefix = args[1] if len(args) > 1 else "/"
    setup_engine = ComarSetup(prefix)

    if command == "install":
        if os.getuid() != 0 and prefix == "/":
            print("[!] HATA: Sistem kurulumu için root yetkisi gereklidir.")
            sys.exit(1)
        setup_engine.install()
    elif command == "dist":
        setup_engine.make_dist() # Zstd mühürlü paket oluşturma
