/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sound_internal.h"

void zvb_sound_set_volume(sound_volume_t vol)
{
    zvb_sound_map();
    zvb_sound_set_volumes(vol, vol);
}

void zvb_sound_set_volumes(sound_volume_t left, sound_volume_t right)
{
    uint8_t val = (left == VOL_0) ? 0x40 : left;
    val |=  (right == VOL_0) ? 0x80 : (right << 2);
    zvb_peri_sound_master_vol = val;
}
