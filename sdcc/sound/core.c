/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sound_internal.h"

void zvb_sound_initialize(uint8_t reset)
{
    zvb_sound_map();
    if (reset) {
        zvb_peri_sound_select = VOICE0 | VOICE1 | VOICE2 | VOICE3;
        zvb_peri_sound_hold = 0xff;

        /* By default, set both channels volume to 100% and assign all voices to both channels */
        zvb_peri_sound_volume = ZVB_PERI_SOUND_VOL_100;
        zvb_peri_sound_left_channel = VOICE0 | VOICE1 | VOICE2 | VOICE3;
        zvb_peri_sound_right_channel = VOICE0 | VOICE1 | VOICE2 | VOICE3;
        zvb_peri_sound_master_vol = ZVB_PERI_SOUND_VOL_DISABLE;

        zvb_peri_sound_select = 0;
    }
}

void zvb_sound_reset(void)
{
    zvb_sound_initialize(1);
}
