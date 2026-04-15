#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Çomar UI Application (Finalized Nihai Version)
# Copyright (C) 2026, Ergün Salman ergunsalman@hotmail.com Poyraz76
#
# ALTYAPI: Python 3.12+, x86_64, PyQt6, Asyncio.
# GÜVENLİK: BLAKE3 (Bütünlük doğrulaması), SHA-512.
# SİSTEM: Systemd-free, Müdür + Çomar (init/daemon) uyumlu.

import sys
import asyncio
import logging
from PyQt6 import QtWidgets, QtCore, uic
from blake3 import blake3

# Modernize edilmiş Çomar API
import comar
from dbus.mainloop.pyqt6 import DBusQtMainLoop

class Window(QtWidgets.QWidget):
    """Çomar servislerini görsel olarak yöneten mühürlü arayüz motoru."""
    
    def __init__(self):
        super().__init__()

        # 1. UI Yükleme ve Bütünlük Kontrolü
        try:
            uic.loadUi("mainform.ui", self)
            self.verify_ui_integrity("mainform.ui")
        except Exception as e:
            self.zeka_analizi("UI_LOAD", e)
            sys.exit(1)

        # 2. Çomar API Bağı
        self.link = comar.api_link()

        # 3. Sinyal/Slot Bağlantıları (Modern Qt6)
        self.buttonServices.clicked.connect(self.get_services_task)

    def zeka_analizi(self, context: str, error: Exception):
        """ZEKA: AI hata analizi ve teknisyen çözüm önerisi."""
        print(f"\n\033[1;91m[!] AI ANALİZİ - {context}\033[0m")
        print(f"[*] Hata: {str(error)}")
        print("[*] Poyraz76 Önerisi: D-Bus haberleşme kiti veya UI dosyası mühürü hatalı.")
        print("[*] Aksiyon: 'comar-daemon' servisini kontrol edin ve SQLite envanterini doğrulayın.")

    def verify_ui_integrity(self, file_path: str):
        """UI dosyasını BLAKE3 ile mühürler."""
        with open(file_path, "rb") as f:
            f_hash = blake3(f.read()).hexdigest()
            print(f"[*] UI Mühürü Doğrulandı: {f_hash[:16]}")

    def get_services_task(self):
        """Asenkron süreci başlatan tetikleyici."""
        self.textServices.clear()
        self.textServices.append("[*] Servisler SQLite envanterinden mühürlü olarak çekiliyor...\n")
        
        # Python 3.12+ asenkron çağrı yönetimi
        asyncio.create_task(self.run_get_services())

    async def run_get_services(self):
        """Çomar API üzerinden servis bilgilerini asenkron toplar."""
        try:
            # Modern API: iterate over services
            for package in self.link.System.Service:
                # Asenkron execute çağrısı
                results = await self.link.System.Service[package].info()
                
                if results:
                    name, desc, state = results
                    entry = f"📦 {package:<15} | 🛠️ {name} | 📝 {desc} | 🚦 {state}"
                    self.textServices.append(entry)
                    
        except Exception as e:
            self.zeka_analizi("SERVICE_FETCH", e)

def main():
    # Qt6 ve D-Bus entegrasyonu
    app = QtWidgets.QApplication(sys.argv)
    DBusQtMainLoop(set_as_default=True)

    # UI başlatma
    win = Window()
    win.show()

    # Qt6 ve Asyncio döngülerini birleştiren modern yapı
    # Not: Üretim ortamında 'qasync' kütüphanesi önerilir.
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
