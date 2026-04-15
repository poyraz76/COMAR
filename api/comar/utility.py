#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Utility Functions (Nihai Sürüm)
# Copyright (C) 2006-2009 TUBITAK/UEKAE
# Copyright (C) 2026, Ergün Salman
#
# ALTYAPI: Python 3.12+, x86_64, SQLite DB, Asyncio.
# GÜVENLİK: BLAKE3 (Mühürleme), SHA-512 (Doğrulama).
# ARŞİVLEME: Zstd (Log ve Çıktı Yönetimi).

import os
import sys
import fcntl
import asyncio
import sqlite3
import logging
import time
import subprocess
import zstandard as zstd
from pathlib import Path
from blake3 import blake3

# --- 2026 Standartları ve Sabitler ---
DB_PATH = Path("/var/lib/pisi/inventory.db")
LOCK_DIR = Path("/run/lock/subsys")

class ExecReply(int):
    """Modernize edilmiş komut yanıt objesi."""
    def __new__(cls, value, stdout=None, stderr=None):
        instance = super(ExecReply, cls).__new__(cls, value)
        instance.stdout = stdout
        instance.stderr = stderr
        instance.timestamp = time.time()
        return instance

def ai_analyze_utility(context: str, error: Exception):
    """ZEKA: AI hata analizi ve teknisyen çözüm önerisi."""
    print(f"\n\033[1;91m[!] AI ANALİZİ - Utility Hatası\033[0m")
    print(f"[*] Bağlam: {context}")
    print(f"[*] Teknik Detay: {str(error)}")
    print("[*] Önerisi: Yetki veya kilit dosyası (FileLock) tıkanıklığı olabilir.")

class FileLock:
    """Bağlamsal (Context Manager) dosya kilitleme motoru."""
    def __init__(self, path: Path, timeout: float = 10.0):
        self.path = Path(path)
        self.timeout = timeout
        self.fd = None

    async def __aenter__(self):
        start_time = time.time()
        if not self.path.parent.exists():
            self.path.parent.mkdir(parents=True, exist_ok=True)
        
        self.fd = os.open(self.path, os.O_WRONLY | os.O_CREAT, 0o600)
        
        while True:
            try:
                # Bloklamayan kilit denemesi
                fcntl.flock(self.fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
                return self
            except (IOError, OSError):
                if time.time() - start_time > self.timeout:
                    os.close(self.fd)
                    raise TimeoutError(f"Kilit alınamadı: {self.path}")
                await asyncio.sleep(0.1)

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.fd:
            fcntl.flock(self.fd, fcntl.LOCK_UN)
            os.close(self.fd)

def get_hash(path: Path) -> str:
    """Dosyanın BLAKE3 mühürünü döndürür."""
    return blake3(path.read_bytes()).hexdigest()

async def execute(command: list, env: dict = None):
    """Komutları asenkron ve güvenli şekilde çalıştırır."""
    try:
        process = await asyncio.create_subprocess_exec(
            *command,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            env=env or os.environ
        )
        stdout, stderr = await process.communicate()
        return ExecReply(
            process.returncode,
            stdout.decode().strip(),
            stderr.decode().strip()
        )
    except Exception as e:
        ai_analyze_utility("Command Execution", e)
        return ExecReply(1, "", str(e))

def compress_log(data: str) -> bytes:
    """Log verisini Zstd ile sıkıştırır."""
    cctx = zstd.ZstdCompressor(level=3)
    return cctx.compress(data.encode())
