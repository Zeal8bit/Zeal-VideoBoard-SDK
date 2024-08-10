#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: CC0-1.0
#

SHELL := /bin/bash

# Specify the files to compile and the name of the final binary
SRCS=snake.c controller.c title.c
BIN=snake.bin

# Directory where source files are and where the binaries will be put
INPUT_DIR=src
OUTPUT_DIR=bin

# Include directory containing Zeal 8-bit OS header files.
ifndef ZOS_PATH
$(error "Please define ZOS_PATH environment variable. It must point to Zeal 8-bit OS source code path.")
endif
ifndef ZVB_SDK_PATH
$(error "Please define ZVB_SDK_PATH environment variable. It must point to Zeal Video Board SDK path.")
endif
ZVB_INCLUDE=$(ZVB_SDK_PATH)/include/
ZOS_INCLUDE=$(ZOS_PATH)/kernel_headers/sdcc/include/
ZVB_LIB_PATH=$(ZVB_SDK_PATH)/lib/
ZOS_LIB_PATH=$(ZOS_PATH)/kernel_headers/sdcc/lib
# Regarding the linking process, we will need to specify the path to the crt0 REL file.
# It contains the boot code for C programs as well as all the C functions performing syscalls.
CRT_REL=$(ZOS_PATH)/kernel_headers/sdcc/bin/zos_crt0.rel


# Compiler, linker and flags related variables
CC=sdcc
# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
# (_CODE must be replace).
CFLAGS=-mz80 --opt-code-speed -c --codeseg TEXT -I$(ZOS_INCLUDE) -I$(ZVB_INCLUDE)
LD=sdldz80
# Make sure the whole program is relocated at 0x4000 as request by Zeal 8-bit OS.
LDFLAGS=-n -mjwx -i -b _HEADER=0x4000 -k $(ZOS_LIB_PATH) -l z80 -k $(ZVB_LIB_PATH) -l zvb_gfx
# Binary used to convert ihex to binary
OBJCOPY=objcopy

# Generate the intermediate Intel Hex binary name
BIN_HEX=$(patsubst %.bin,%.ihx,$(BIN))
# Generate the rel names for C source files. Only keep the file names, and add output dir prefix.
SRCS_OUT_DIR=$(addprefix $(OUTPUT_DIR)/,$(SRCS))
SRCS_REL=$(patsubst %.c,%.rel,$(SRCS_OUT_DIR))
SRCS_ASM_REL=$(patsubst %.asm,%.rel,$(SRCS_OUT_DIR))


.PHONY: all clean

all: clean $(OUTPUT_DIR) $(OUTPUT_DIR)/$(BIN_HEX) $(OUTPUT_DIR)/$(BIN)
	@bash -c 'echo -e "\x1b[32;1mSuccess, binary generated: $(OUTPUT_DIR)/$(BIN)\x1b[0m"'

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Generate a REL file for each source file. In fact, SDCC doesn't support compiling multiple source file
# at once. We have to create the same directory structure in output dir too.
$(SRCS_REL): $(OUTPUT_DIR)/%.rel : $(INPUT_DIR)/%.c
	@mkdir -p $(OUTPUT_DIR)/$(dir $*)
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/$(dir $*) $<

# Generate the final Intel HEX binary.
$(OUTPUT_DIR)/$(BIN_HEX): $(CRT_REL) $(SRCS_REL)
	$(LD) $(LDFLAGS) $(OUTPUT_DIR)/$(BIN_HEX) $(CRT_REL) $(SRCS_REL)

# Convert the Intel HEX file to an actual binary.
$(OUTPUT_DIR)/$(BIN):
	$(OBJCOPY) --input-target=ihex --output-target=binary $(OUTPUT_DIR)/$(BIN_HEX) $(OUTPUT_DIR)/$(BIN)

clean:
	rm -fr bin/