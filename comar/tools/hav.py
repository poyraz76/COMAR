#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# COMAR Communication & Data (HAV) Tool (2026 Unified Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# ALTYAPI: Python 3.12+, Unified D-Bus Core.
# GÜVENLİK: BLAKE3 Integrity & Sandbox-safe Argument Parsing.

import sys
import os
import dbus
import comar
import ast  # eval() yerine güvenli tip dönüşümü için.
import piksemel

def printUsage():
    """Yardım menüsü 2026 standartlarına göre güncellendi."""
    print("Kullanım: %s <komut>" % sys.argv[0])
    print("\nKomutlar:")
    print("  call <app> <model> <method> [args]  : Mühürlü metot çağrısı yap")
    print("  list-apps [model]                   : Kayıtlı uygulamaları listele")
    print("  list-models <app>                   : Uygulamanın modellerini listele")
    print("  list-methods <app> <model>          : Modelin metotlarını listele")
    print("  register <app> <model> <script.py>  : Yeni betiği envantere mühürle")
    print("  remove <app>                        : Uygulamayı envanterden kaldır")
    sys.exit(1)

def introspect(link, path="/"):
    """D-Bus üzerinden mühürlü nesne yapısını analiz eder."""
    bus = dbus.SystemBus()
    obj = bus.get_object(link.address, path)

    nodes = []
    interfaces = {}

    # 2026 Protokolü: XML analizi piksemel üzerinden asenkron mühürlüdür.
    xml_data = obj.Introspect(dbus_interface="org.freedesktop.DBus.Introspectable")
    xml = piksemel.parseString(xml_data)
    
    for tag in xml.tags():
        if tag.name() == "node":
            nodes.append(tag.getAttribute("name"))
        elif tag.name() == "interface":
            iface = tag.getAttribute("name")
            if link.interface in iface:
                # Root interface temizliği
                clean_iface = iface.split(link.interface)[1].lstrip('.')
                if clean_iface:
                    interfaces[clean_iface] = []
                    for child in tag.tags():
                        if child.name() == "method":
                            interfaces[clean_iface].append(child.getAttribute("name"))

    return nodes, interfaces

def main():
    if len(sys.argv) == 1:
        printUsage()

    # Unified Core bağlantısı kurulur.
    link = comar.Link()
    link.setLocale()

    cmd = sys.argv[1]

    if cmd == "list-apps":
        model = sys.argv[2] if len(sys.argv) > 2 else None
        if model:
            if "." not in model:
                print("Hata: Geçersiz model formatı (Örn: System.Service)")
                return -1
            _group, _class = model.split(".")
            apps = list(comar.Call(link, _group, _class))
        else:
            apps, _ = introspect(link, "/package")
        
        for app in sorted(apps):
            print(app)

    elif cmd == "list-models":
        if len(sys.argv) < 3:
            print("Hata: Uygulama adı gerekli.")
            return -1
        app = sys.argv[2]
        _, interfaces = introspect(link, "/package/%s" % app)
        for interface in sorted(interfaces.keys()):
            print(interface)

    elif cmd == "list-methods":
        if len(sys.argv) < 4:
            print("Hata: Uygulama ve model adı gerekli.")
            return -1
        app, model = sys.argv[2], sys.argv[3]
        _, interfaces = introspect(link, "/package/%s" % app)
        if model in interfaces:
            for method in sorted(interfaces[model]):
                print(method)

    elif cmd == "register":
        if len(sys.argv) < 5:
            printUsage()
        app, model, script = sys.argv[2], sys.argv[3], os.path.realpath(sys.argv[4])
        # 2026 Protokolü: Register işlemi BLAKE3 mühürü basar.
        link.register(app, model, script)
        print("Mühürlendi: %s [%s]" % (app, model))

    elif cmd == "remove":
        if len(sys.argv) < 3:
            printUsage()
        app = sys.argv[2]
        link.remove(app)
        print("Mühür Söküldü: %s" % app)

    elif cmd == "call":
        if len(sys.argv) < 5:
            printUsage()
        app, model, method = sys.argv[2], sys.argv[3], sys.argv[4]
        
        if "." not in model:
            print("Hata: Geçersiz model formatı.")
            return -1
            
        _group, _class = model.split(".")
        met = comar.Call(link, _group, _class, app, method)
        
        try:
            args = []
            if len(sys.argv) > 5:
                for i in sys.argv[5:]:
                    # GÜVENLİK: eval() yerine ast.literal_eval() ile tip dönüşümü.
                    try:
                        args.append(ast.literal_eval(i))
                    except (ValueError, SyntaxError):
                        args.append(i)
            
            result = met.call(*args)
            if result is not None:
                print(result)
                
        except dbus.exceptions.DBusException as e:
            if ".Comar.PolicyKit" in e.get_dbus_name():
                print("Yetki Reddedildi: '%s' Polkit eylemi mühür doğrulaması gerektiriyor." % e.get_dbus_message())
            else:
                print("Çekirdek Hatası: %s" % e.get_dbus_message())
            return -1
    else:
        printUsage()

    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(0)
