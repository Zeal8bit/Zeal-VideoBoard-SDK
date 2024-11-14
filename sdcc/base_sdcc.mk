#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: Apache-2.0
#

# This file is means to be included by programs based on SDCC.
# This will ease writing a Makefile for a new project that is meant to use ZVB SDK.
# This file can included by adding this line to any Makefile:
#	include $(ZVB_SDK_PATH)/sdcc/base_sdcc.mk

ifndef ZOS_PATH
    $(error "Failure: ZOS_PATH variable not found. It must point to Zeal 8-bit OS path.")
endif
ifndef ZVB_SDK_PATH
    $(error "Failure: ZVB_SDK_PATH variable not found. It must point to Zeal Video Board SDK path.")
endif

# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
ZVB_CFLAGS ?= -I$(ZVB_SDK_PATH)/include/
ZOS_CFLAGS += $(ZVB_CFLAGS)
ENABLE_GFX ?= 1
ENABLE_SOUND ?= 0
ENABLE_CRC32 ?= 0

# Make sure the whole program is relocated at 0x4000 as request by Zeal 8-bit OS.
ZVB_LDFLAGS ?= -k $(ZVB_SDK_PATH)/lib/

ifeq ($(ENABLE_GFX)), 1)
 ZVB_FLAGS += -l zvb_gfx
 endif

ifeq ($(ENABLE_SOUND), 1)
ZVB_LDFLAGS += -l zvb_sound
endif

ifeq ($(ENABLE_CRC32), 1)
ZVB_LDFLAGS += -l zvb_crc32
endif

ZOS_LDFLAGS += $(ZVB_LDFLAGS)


include $(ZOS_PATH)/kernel_headers/sdcc/base_sdcc.mk