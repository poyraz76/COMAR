#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Communication & Data (HAV) Tool (2026 Unified Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# ALTYAPI: Python 3.12+, Unified D-Bus Core.
# GÜVENLİK: Polkit Authorization & BLAKE3 Integrity Awareness.

import dbus
import locale
import sys
import os

# 2026 Unified Core Adreslemeleri
COMAR_BUS = 'tr.org.pardus.comar'
COMAR_PATH = '/'

def handleError(exception):
    """Hataları 2026 teşhis protokolüne göre işler."""
    error_name = exception.get_dbus_name()
    message = exception.get_dbus_message()
    
    if "Comar.PolicyKit" in error_name:
        # [span_1](start_span)Polkit yetki reddi durumunda eylem kimliğini basar[span_1](end_span)
        print(f"❌ Yetki Reddedildi: '{message}' eylemi için mühürlü izin gerekiyor.")
    else:
        print(f"⚠️ Çekirdek Hatası: {message}")
    sys.exit(1)

def printUsage():
    """Yardım menüsü 2026 standartlarına göre güncellendi."""
    print(f"Kullanım: {sys.argv[0]} <komut>")
    print("\nKomutlar:")
    print("  list-apps <model>                  : Modelin kayıtlı uygulamalarını listele")
    print("  list-models <app>                  : Uygulamanın desteklediği modelleri listele")
    print("  register <app> <model> <script.py> : Yeni betiği envantere mühürle")
    print("  remove <app>                       : Uygulamanın mühürlerini sök ve kaldır")
    sys.exit(1)

def main():
    if len(sys.argv) == 1:
        printUsage()

    try:
        # Sistem veri yoluna bağlan ve Unified nesneyi al
        bus = dbus.SystemBus()
        obj = bus.get_object(COMAR_BUS, COMAR_PATH, introspect=False)
        
        # Yerel dil ayarını çekirdeğe mühürle
        try:
            lang = locale.getdefaultlocale()[0].split("_")[0]
        except (AttributeError, IndexError):
            lang = "en"
        obj.setLocale(lang, dbus_interface=COMAR_BUS)

        cmd = sys.argv[1]

        # 1. [span_2](start_span)Uygulama Listeleme (Model bazlı)[span_2](end_span)
        if cmd == "list-apps":
            if len(sys.argv) < 3: printUsage()
            apps = obj.listModelApplications(sys.argv[2], dbus_interface=COMAR_BUS)
            for app in sorted(apps):
                print(app)

        # 2. [span_3](start_span)Model Listeleme (Uygulama bazlı)[span_3](end_span)
        elif cmd == "list-models":
            if len(sys.argv) < 3: printUsage()
            models = obj.listApplicationModels(sys.argv[2], dbus_interface=COMAR_BUS)
            for model in sorted(models):
                print(model)

        # 3. [span_4](start_span)Mühürlü Kayıt (Register)[span_4](end_span)
        elif cmd == "register":
            if len(sys.argv) < 5: printUsage()
            app, model, script = sys.argv[2], sys.argv[3], os.path.realpath(sys.argv[4])
            # BLAKE3 mühürleme süreci çekirdek tarafında tetiklenir
            obj.register(app, model, script, dbus_interface=COMAR_BUS)
            print(f"✅ {app} uygulaması {model} modeliyle mühürlendi.")

        # 4. [span_5](start_span)Kayıt Silme (Remove)[span_5](end_span)
        elif cmd == "remove":
            if len(sys.argv) < 3: printUsage()
            obj.remove(sys.argv[2], dbus_interface=COMAR_BUS)
            print(f"🗑️ {sys.argv[2]} uygulamasının mühürleri söküldü.")

        else:
            printUsage()

    except dbus.DBusException as e:
        handleError(e)
    except Exception as e:
        print(f"Beklenmeyen Hata: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
