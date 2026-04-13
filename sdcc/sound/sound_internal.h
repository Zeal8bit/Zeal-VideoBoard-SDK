/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "zvb_sound.h"

#define BIT(n)  (1 << (n))

extern uint8_t zvb_sound_master_hold;

static inline void zvb_sound_map(void)
{
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}
