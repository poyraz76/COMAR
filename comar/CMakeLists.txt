# COMAR Unified Core Build Configuration (2026 Edition)
# Copyright (c) 2026, Ergün Salman (Poyraz76)
#
# [span_3](start_span)ALTYAPI: Modern CMake 3.12+, Python 3.12+, BLAKE3 Integrity.[span_3](end_span)
# [span_4](start_span)SİSTEM: Unified "tr.org.pardus.comar" Namespace.[span_4](end_span)

# [span_5](start_span)Proje tanımı mühürlendi.[span_5](end_span)
PROJECT (comar)

# [span_6](start_span)2026 Standartları için modern CMake şarttır.[span_6](end_span)
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

# [span_7](start_span)Python3 modern paket keşfi.[span_7](end_span)
FIND_PACKAGE(Python3 COMPONENTS Development REQUIRED)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES (DBUS REQUIRED dbus-1)

# [span_8](start_span)Uygulama ismi mühürlendi; artık comar2/3/updated takıları yok.[span_8](end_span)
SET (APP_NAME "comar")

# [span_9](start_span)Derleme bayrakları 2026 güvenlik protokolüne göre sıkılaştırıldı.[span_9](end_span)
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# [span_10](start_span)Dizin tanımlamaları 2026 Unified Core yapısına göre güncellendi.[span_10](end_span)
ADD_DEFINITIONS (
    -D'PID_FILE="/run/${APP_NAME}.pid"'
    -D'DATA_DIR="/var/lib/${APP_NAME}"'
    -D'LOG_DIR="/var/log/${APP_NAME}"'
    -D'VERSION="4.0.0"'
)

# [span_11](start_span)Çekirdek kaynak dosyalar.[span_11](end_span)
SET (SOURCES 
    src/main.c
    src/bus.c
    src/config.c
    src/db.c
    src/iksemel.c
    src/log.c
    src/loop.c
    src/process.c
    src/policy.c
    src/pydbus.c
    src/script.c
    src/utils.c
)

# [span_12](start_span)Kütüphane bağları modern Python3 ve D-Bus ile mühürlendi.[span_12](end_span)
SET (LIBS 
    Python3::Python
    ${DBUS_LIBRARIES}
)

# [span_13](start_span)Başlık dosyası dizinleri.[span_13](end_span)
INCLUDE_DIRECTORIES (
    include/
    ${Python3_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
)

# [span_14](start_span)İcra edilebilir dosyanın oluşturulması.[span_14](end_span)
ADD_EXECUTABLE (comar ${SOURCES})

# [span_15](start_span)Kütüphane linkleme.[span_15](end_span)
TARGET_LINK_LIBRARIES(comar ${LIBS})

# --- KURULUM (INSTALL) PROTOKOLÜ ---

# 1. [span_16](start_span)Ana çekirdek /usr/sbin altına kurulur.[span_16](end_span)
INSTALL (TARGETS comar DESTINATION /usr/sbin)

# 2. [span_17](start_span)Model ve Modüller mühürlü SQLite/Inventory yapısı için /var/lib'e taşındı.[span_17](end_span)
INSTALL (DIRECTORY models DESTINATION /var/lib/${APP_NAME})
INSTALL (DIRECTORY modules DESTINATION /var/lib/${APP_NAME})

# 3. [span_18](start_span)D-Bus Servis Aktivasyonu (Sadece ana servis kurulur).[span_18](end_span)
INSTALL (FILES config/tr.org.pardus.comar.service 
         DESTINATION /usr/share/dbus-1/system-services/)

# 4. [span_19](start_span)Polkit Güvenlik Politikaları.[span_19](end_span)
INSTALL (DIRECTORY policy/ 
         DESTINATION /usr/share/polkit-1/actions
         FILES_MATCHING PATTERN "*.policy")

# 5. [span_20](start_span)D-Bus Sistem Konfigürasyonu.[span_20](end_span)
INSTALL (FILES config/tr.org.pardus.comar.conf 
         DESTINATION /etc/dbus-1/system.d)

# 6. [span_21](start_span)Yardımcı Araçlar (Sadece aktif hav aracı kurulur).[span_21](end_span)
INSTALL (PROGRAMS tools/hav.py DESTINATION /usr/bin RENAME hav)
