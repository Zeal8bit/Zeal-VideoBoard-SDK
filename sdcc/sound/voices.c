/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sound_internal.h"

uint8_t zvb_sound_master_hold;

void zvb_sound_set_voices(sound_voice_t voices, uint16_t divider, sound_waveform_t waveform)
{
    zvb_sound_map();

    /* Temporarily hold the voices */
    zvb_peri_sound_hold = zvb_sound_master_hold | voices;

    /* Map the voices */
    zvb_peri_sound_select = voices;
    /* Set the divider and the waveform */
    zvb_peri_sound_freq_low  = divider & 0xff;
    zvb_peri_sound_freq_high = (divider >> 8) & 0xff;
    zvb_peri_sound_wave = waveform;
    /* Unhold the voices if they were unhold before the call */
    zvb_peri_sound_hold = zvb_sound_master_hold;
}

void zvb_sound_set_voices_vol(sound_voice_t voices, sound_volume_t vol)
{
    zvb_sound_map();
    zvb_peri_sound_select = voices;
    zvb_peri_sound_volume = vol;
}

void zvb_sound_set_hold(sound_voice_t voices, uint8_t hold)
{
    zvb_sound_map();
    if (hold == 0) {
        zvb_sound_master_hold &= ~voices;
    } else {
        zvb_sound_master_hold |= voices;
    }
    zvb_peri_sound_hold = zvb_sound_master_hold;
}

sound_voice_t zvb_sound_get_hold(void)
{
    return zvb_sound_master_hold;
}
