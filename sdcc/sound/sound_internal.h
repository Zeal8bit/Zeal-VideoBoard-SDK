/**
 * SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_sound.h"

#define BIT(n)  (1 << (n))

#define zvb_sound_map() zvb_map_peripheral(ZVB_PERI_SOUND_IDX)
