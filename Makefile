#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: Apache-2.0
#

SHELL := /bin/bash

INPUT_DIR=sdcc
OUTPUT_DIR=lib

ifndef ZVB_SDK_PATH
$(error "Please define ZVB_SDK_PATH environment variable. It must point to Zeal Video Board SDK path.")
endif
ZVB_INCLUDE=$(ZVB_SDK_PATH)/include/

CC=sdcc
AR=sdar
# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
# (_CODE must be replace).
CFLAGS=-mz80 -c --codeseg TEXT -I$(ZVB_INCLUDE) --opt-code-speed

.PHONY: all clean

all: $(OUTPUT_DIR) $(OUTPUT_DIR)/zvb_gfx.lib $(OUTPUT_DIR)/zvb_crc.lib
	@bash -c 'echo -e "\x1b[32;1mSuccess, libraries generated\x1b[0m"'

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/zvb_gfx.lib: $(INPUT_DIR)/zvb_gfx.c
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/ $^
	$(AR) -rc $@ $(patsubst $(INPUT_DIR)/%.c,$(OUTPUT_DIR)/%.rel,$^)


$(OUTPUT_DIR)/zvb_crc.lib: $(INPUT_DIR)/zvb_crc.c
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/ $^
	$(AR) -rc $@ $(patsubst $(INPUT_DIR)/%.c,$(OUTPUT_DIR)/%.rel,$^)

clean:
	rm -f lib/*