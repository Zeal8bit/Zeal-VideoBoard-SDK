/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "zvb_sound.h"

static inline void zvb_sound_map(void)
{
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}

void zvb_sound_initialize(uint8_t reset)
{
    zvb_sound_map();
    if (reset) {
        zvb_peri_sound_master_vol = ZVB_PERI_SOUND_VOL_DISABLE;
        zvb_peri_sound_select = 0;
        zvb_peri_sound_hold = 0xff;
    }
}


void zvb_sound_reset(void)
{
    zvb_sound_initialize(1);
}


void zvb_sound_set_voices(sound_voice_t voices, uint16_t divider, sound_waveform_t waveform)
{
    zvb_sound_map();

    sound_voice_t state = zvb_peri_sound_hold;
    zvb_sound_set_hold(voices, 1);

    /* Map the voices */
    zvb_peri_sound_select = voices;
    /* Set the divider and the waveform */
    zvb_peri_sound_freq_low  = divider & 0xff;
    zvb_peri_sound_freq_high = (divider >> 8) & 0xff;
    zvb_peri_sound_wave = waveform;
    /* Unhold the voices if they were unhold before the call */
    zvb_peri_sound_hold = state;
}


sound_voice_t zvb_sound_get_hold(void)
{
    zvb_sound_map();
    return zvb_peri_sound_hold;
}

void zvb_sound_set_hold(sound_voice_t voices, uint8_t hold)
{
    zvb_sound_map();
    if (hold == 0) {
        zvb_peri_sound_hold &= ~voices;
    } else {
        zvb_peri_sound_hold |= voices;
    }
}

sound_volume_t zvb_sound_get_volume(void)
{
    zvb_sound_map();
    return zvb_peri_sound_master_vol;
}

void zvb_sound_set_volume(sound_volume_t vol)
{
    zvb_sound_map();
    zvb_peri_sound_master_vol = vol;
}