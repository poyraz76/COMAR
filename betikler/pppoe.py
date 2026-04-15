#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR PPPoE Configuration Module (Nihai Sürüm)
# Copyright (C) 2005-2009 TUBITAK/UEKAE
# Copyright (C) 2026, Ergün Salman
#
# [span_3](start_span)ALTYAPI: Python 3.12+, x86_64, SQLite DB, Asyncio.[span_3](end_span)
# [span_4](start_span)GÜVENLİK: BLAKE3 (Konfigürasyon bütünlük mühürü).[span_4](end_span)
# [span_5](start_span)SİSTEM: rp-pppoe uyumlu, COMAR Envanter entegrasyonu.[span_5](end_span)

import os
import asyncio
import sqlite3
import logging
from pathlib import Path
from blake3 import blake3
[span_6](start_span)from comar.utility import execute #[span_6](end_span)

# --- 2026 Standartları ---
DB_PATH = Path("/var/lib/pisi/inventory.db")
CONF_BASE = Path("/etc/ppp")

class PPPoEHandler:
    """Ethernet üzerinden PPP bağlantılarını asenkron yöneten motor."""

    tmpl_pppoe_conf = """# COMAR 2026 PPPoE Config
ETH="%s"
USER="%s"
DEMAND=no
DNSTYPE=COMAR
PEERDNS=yes
DEFAULTROUTE=yes
CONNECT_TIMEOUT=30
CONNECT_POLL=2
PIDFILE="/var/run/adsl.pid"
SYNCHRONOUS=no
CLAMPMSS=1412
LCP_INTERVAL=20
LCP_FAILURE=3
PPPOE_TIMEOUT=80
FIREWALL=NONE
"""
    tmpl_options = "noipdefault\nhide-password\ndefaultroute\npersist\nlock\n"

    def __init__(self):
        self.logger = logging.getLogger("Network.PPPoE")

    def zeka_analizi(self, context: str, error: str):
        [span_7](start_span)[span_8](start_span)"""ZEKA: AI hata analizi ve teknisyen çözüm önerisi.[span_7](end_span)[span_8](end_span)"""
        print(f"\n\033[1;91m[!] AI ANALİZİ - PPPoE Hatası: {context}\033[0m")
        print(f"[*] Detay: {error}")
        print(f"[*] Ergün, /etc/ppp dizin izinlerini ve kablo bağlantısını kontrol etmeni öneririm.")

    def _update_inventory(self, path: Path):
        [span_9](start_span)[span_10](start_span)"""Dosyayı BLAKE3 ile mühürleyip SQLite envanterine işler.[span_9](end_span)[span_10](end_span)"""
        try:
            f_hash = blake3(path.read_bytes()).hexdigest()
            with sqlite3.connect(DB_PATH) as conn:
                conn.execute("INSERT OR REPLACE INTO file_inventory (path, hash, timestamp) VALUES (?, ?, CURRENT_TIMESTAMP)", 
                            (str(path), f_hash))
        except Exception as e:
            self.logger.debug(f"Mühürleme hatası: {e}")

    async def create_configs(self, dev: str, user: str, pwd: str):
        [span_11](start_span)"""Yapılandırma dosyalarını mühürlü olarak oluşturur.[span_11](end_span)"""
        try:
            # 1. pppoe.conf
            conf = CONF_BASE / "pppoe.conf"
            conf.write_text(self.tmpl_pppoe_conf % (dev, user))
            self._update_inventory(conf)

            # 2. options-pppoe
            opts = CONF_BASE / "options-pppoe"
            opts.write_text(self.tmpl_options)
            self._update_inventory(opts)

            # 3. Secrets (Güvenli 0600 izni ile)
            secrets = CONF_BASE / "pap-secrets"
            if secrets.exists(): secrets.unlink()
            
            # [span_12](start_span)Python 3.12 güvenli dosya açma[span_12](end_span)
            fd = os.open(secrets, os.O_CREAT | os.O_WRONLY, 0o600)
            with os.fdopen(fd, 'w') as f:
                f.write(f'"{user}" * "{pwd}"\n')
            self._update_inventory(secrets)
            
            # Chap secrets linkleme
            chap = CONF_BASE / "chap-secrets"
            chap.unlink(missing_ok=True)
            chap.symlink_to(secrets)
            
            return True
        except Exception as e:
            self.zeka_analizi("Yapılandırma Oluşturma", str(e))
            return False

    async def connect(self, dev: str, user: str, pwd: str):
        [span_13](start_span)[span_14](start_span)"""Bağlantıyı asenkron başlatır ve envanteri günceller.[span_13](end_span)[span_14](end_span)"""
        if await self.create_configs(dev, user, pwd):
            self.logger.info(f"PPPoE Başlatılıyor: {user}@{dev}")
            [span_15](start_span)reply = await execute(["/usr/sbin/adsl-start"])[span_15](end_span)
            
            with sqlite3.connect(DB_PATH) as conn:
                status = "online" if reply == 0 else "failed"
                conn.execute("INSERT OR REPLACE INTO network_status (interface, pppoe_state) VALUES (?, ?)", (dev, status))
            
            return reply.stdout
        return "Yapılandırma başarısız."

    async def stop(self):
        """Bağlantıyı güvenli şekilde keser."""
        [span_16](start_span)return await execute(["/usr/sbin/adsl-stop"])[span_16](end_span)

    async def get_status(self):
        """Bağlantı durumunu sorgular."""
        [span_17](start_span)return await execute(["/usr/sbin/adsl-status"])[span_17](end_span)
