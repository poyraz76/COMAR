# ==============================================================================
# Çomar UI Build System (Finalized Nihai Version)
# Copyright (C) 2026, Ergün Salman ergunsalman@hotmail.com Poyraz76
# ==============================================================================
# ALTYAPI: Python 3.12+, PyQt6, x86_64.
# GÜVENLİK: BLAKE3 (Bütünlük doğrulaması ve mühürleme).
# ARŞİVLEME: Zstd (Derleme çıktıları yedekleme).
# ==============================================================================

# --- Değişkenler ---
UIC      = pyuic6
PYTHON   = python3
UI_FILE  = mainform.ui
PY_FILE  = mainform.py
ZSTD     = tar --zstd
VERSION  = 4.0.0

# --- Ana Hedef ---
all: compile seal

# 1. [span_1](start_span)UI Derleme (Qt4 -> Qt6 Dönüşümü)[span_1](end_span)
compile:
	@echo "[*] UI dosyası Qt6 standartlarında derleniyor: $(UI_FILE) -> $(PY_FILE)"
	$(UIC) $(UI_FILE) -o $(PY_FILE)

# 2. Güvenlik Mühürü (BLAKE3 Doğrulaması)
# Üretilen her Python dosyasını anlık olarak mühürler.
seal:
	@echo "[*] Üretilen kod BLAKE3 ile mühürleniyor..."
	@$(PYTHON) -c "from blake3 import blake3; print('Mühür (BLAKE3):', blake3(open('$(PY_FILE)', 'rb').read()).hexdigest())"

# 3. Arşivleme (Zstd Katmanı)
# Derleme çıktılarını 2026 standartlarında paketler.
dist:
	@echo "[*] Dağıtım paketi mühürleniyor (Zstd)..."
	$(ZSTD) -cf comar-ui-$(VERSION).tar.zst $(UI_FILE) $(PY_FILE) Makefile

# 4. Temizlik
clean:
	@echo "[*] Geçici dosyalar temizleniyor..."
	rm -f $(PY_FILE) *.tar.zst

.PHONY: all compile seal dist clean
