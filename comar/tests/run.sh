#!/bin/sh
#
# COMAR Unified Core - Ignition & Sandbox Script (2026 Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# AMAÇ: Çekirdeği derler, sandbox ortamına kurar ve mühürlü modda başlatır.

# 1. Eski İzleri Temizle
rm -rf comar_root

# 2. FHS 2026 Standart Dizinlerini İnşa Et
# comar3 yerine sadece comar; db yerine modern lib kullanıyoruz.
mkdir -p comar_root/var/log/comar
mkdir -p comar_root/var/lib/comar

# 3. İnşa ve Kuruluma Geç (Build & Install)
cd ..
# CMake 3.12+ mühürlü inşa planını tetikle
cmake .
make -j$(nproc)
# Derlenen mühürlü çekirdeği test köküne kur
make install DESTDIR=tests/comar_root
cd tests

echo "\n[✓] COMAR Unified Core 4.0 sandbox ortamına kuruldu."
echo "[!] Çekirdek 'root' yetkisi ve mühürlü parametrelerle başlatılıyor...\n"

# 4. Mühürlü Çekirdeği Ateşle
# --datadir ve --logdir parametreleri modernize edildi.
sudo comar_root/usr/sbin/comar \
    --datadir=comar_root/var/lib/comar \
    --logdir=comar_root/var/log/comar \
    --debug \
    --print
