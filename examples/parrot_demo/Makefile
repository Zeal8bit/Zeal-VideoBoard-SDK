#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: CC0-1.0
#

BIN=parrot.bin

OUTPUT_DIR=bin

ifndef ZVB_SDK_PATH
$(error "Please define ZVB_SDK_PATH environment variable. It must point to Zeal Video Board SDK path.")
endif

all: $(OUTPUT_DIR) src/parrot.asm
	z88dk-z80asm -o$(OUTPUT_DIR)/$(BIN) -I$(ZVB_SDK_PATH)/include/ -m -b src/parrot.asm
	../concat.sh $(OUTPUT_DIR)/$(BIN).img 0x0000 $(OUTPUT_DIR)/$(BIN) 0x4000 $(OUTPUT_DIR)/parrot_TILESET.bin


$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -r bin/ src/*.o