/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include <zvb_sound.h>
#include <zos_sys.h>

#define PARTITION_MAX_LEN   1024

extern uint16_t partition[PARTITION_MAX_LEN];
extern uint16_t partition_length;

zos_err_t parse_notes_file(const char* filename);
