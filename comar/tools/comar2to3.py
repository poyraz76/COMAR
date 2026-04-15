#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Legacy Migrator (2026 Unified Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# AMAÇ: COMAR 2.x/3.x betiklerini Unified Core 4.0 envanterine taşır.
# ALTYAPI: Python 3.12+, Unified D-Bus.

import dbus
import os
import sys
import time
import subprocess

# [span_4](start_span)[span_5](start_span)2026 Standart Yolları[span_4](end_span)[span_5](end_span)
COMAR_DB_OLD = "/var/db/comar/code"
COMAR_DB_NEW = "/var/lib/comar/scripts"
COMAR_BUS_NAME = "tr.org.pardus.comar"
COMAR_TIMEOUT = 10

def main():
    if os.getuid() != 0:
        print("Hata: Bu işlem 'root' yetkisi (mühürleme izni) gerektirir.")
        return -1

    # [span_6](start_span)Hedef envanter zaten mühürlüyse kontrol et[span_6](end_span)
    if os.path.exists(COMAR_DB_NEW) and any(os.scandir(COMAR_DB_NEW)):
        # comar.py hariç bir dosya varsa zaten initialize olmuştur.
        print("Bilgi: Modern COMAR envanteri zaten hazır. Taşıma iptal edildi.")
        return 0

    bus = dbus.SystemBus()
    
    # 2026 Protokolü: Daemon'ı os.system yerine kontrollü subprocess ile başlatıyoruz.
    # -[span_7](start_span)[span_8](start_span)[span_9](start_span)i (init) ve -b (bus) parametreleri mühürlenmiştir[span_7](end_span)[span_8](end_span)[span_9](end_span).
    try:
        subprocess.Popen(["/usr/sbin/comar", "-i", "-b", COMAR_BUS_NAME])
    except FileNotFoundError:
        print("Hata: /usr/sbin/comar bulunamadı. Önce çekirdek kurulumunu yapın.")
        return -2

    # Servisin hazır olmasını bekle (Mühür doğrulama süreci)
    obj = None
    timeout = COMAR_TIMEOUT
    while timeout > 0:
        try:
            obj = bus.get_object(COMAR_BUS_NAME, "/", introspect=False)
            break
        except dbus.exceptions.DBusException:
            pass
        time.sleep(0.5)
        timeout -= 0.5

    if not obj:
        print("Hata: Modern COMAR çekirdeği ayağa kaldırılamadı.")
        return -3

    # [span_10](start_span)Eski betikleri tara ve yeni yapıya mühürle[span_10](end_span)
    if not os.path.exists(COMAR_DB_OLD):
        print("Bilgi: Eski COMAR 2.x envanteri bulunamadı. İşlem tamam.")
        return 0

    for filename in os.listdir(COMAR_DB_OLD):
        if filename.endswith(".py"):
            try:
                # Eski isimlendirme: Group_Class_App.py
                _group, _class, _app = filename.split("_", 2)
                _model = f"{_group}.{_class}"
                _app = _app.rsplit(".py", 1)[0]
                
                script_path = os.path.join(COMAR_DB_OLD, filename)
                
                # Çekirdek üzerinden mühürlü kayıt (Register)
                # [span_11](start_span)[span_12](start_span)[span_13](start_span)Modern COMAR 4.0 interface'i üzerinden çağrı yapılır[span_11](end_span)[span_12](end_span)[span_13](end_span).
                obj.register(_app, _model, script_path, dbus_interface=COMAR_BUS_NAME)
                print(f"Mühürlendi: {filename} -> Modern {_model}")
                
            except (ValueError, dbus.exceptions.DBusException) as e:
                print(f"Uyarı: {filename} taşınırken hata oluştu: {e}")

    print("--- Taşıma Başarıyla Tamamlandı ---")
    return 0

if __name__ == "__main__":
    sys.exit(main())
