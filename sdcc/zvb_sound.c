/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "zvb_sound.h"

#define BIT(n)  (1 << (n))

static uint8_t s_mst_hold;

static inline void zvb_sound_map(void)
{
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}

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


void zvb_sound_set_voices(sound_voice_t voices, uint16_t divider, sound_waveform_t waveform)
{
    zvb_sound_map();

    /* Temporarily hold the voices */
    zvb_peri_sound_hold = s_mst_hold | voices;

    /* Map the voices */
    zvb_peri_sound_select = voices;
    /* Set the divider and the waveform */
    zvb_peri_sound_freq_low  = divider & 0xff;
    zvb_peri_sound_freq_high = (divider >> 8) & 0xff;
    zvb_peri_sound_wave = waveform;
    /* Unhold the voices if they were unhold before the call */
    zvb_peri_sound_hold = s_mst_hold;
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
        s_mst_hold &= ~voices;
    } else {
        s_mst_hold |= voices;
    }
    zvb_peri_sound_hold = s_mst_hold;
}

sound_voice_t zvb_sound_get_hold(void)
{
    return s_mst_hold;
}


void zvb_sound_set_channels(sound_voice_t left_voices, sound_voice_t right_voices)
{
    zvb_sound_map();
    zvb_peri_sound_left_channel = left_voices;
    zvb_peri_sound_right_channel = right_voices;
}

void zvb_sound_set_volume(sound_volume_t vol)
{
    zvb_sound_map();
    zvb_sound_set_volumes(vol, vol);
}


void zvb_sound_set_volumes(sound_volume_t left, sound_volume_t right)
{
    zvb_sound_map();
    uint8_t val = (left == VOL_0) ? 0x40 : left;
    val |=  (right == VOL_0) ? 0x80 : (right << 2);
    zvb_peri_sound_master_vol = val;
}


static uint8_t zvb_sound_int_to_conf(sample_int mode)
{
    if (mode == SAMPLE_UINT8) {
        return BIT(ZVB_SAMPLE_CONF_SIZE_BIT);
    } else if (mode == SAMPLE_SINT16) {
        return BIT(ZVB_SAMPLE_CONF_SIGN_BIT);
    } else {
        return 0;
    }
}


static void zvb_sound_wait_empty(void)
{
    while ((zvb_peri_sound_sample_conf & BIT(ZVB_SAMPLE_CONF_READY_BIT)) == 0) {
    }
}

static uint8_t zvb_sound_fifo_full(void)
{
    return (zvb_peri_sound_sample_conf & BIT(ZVB_SAMPLE_CONF_FULL_BIT)) != 0;
}


void zvb_sound_play_samples(sound_samples_conf_t* config, void* samples, uint16_t length)
{
    if (config == NULL || samples == NULL || length == 0) {
        return;
    }
    /* Map the sound controller and enable the sample table voice */
    zvb_sound_map();
    zvb_peri_sound_select = SAMPTAB;

    /* Configure the sign and bit-width of each sample, as well as the sample rate divider */
    zvb_peri_sound_sample_conf = zvb_sound_int_to_conf(config->mode);
    zvb_peri_sound_sample_div = config->divider;

    /* Unhold sample table if it is held */
    zvb_peri_sound_hold = s_mst_hold & ~SAMPTAB;

    /* Play all the samples */
    uint8_t* data = (uint8_t*) samples;

    while (length > 0) {
        if (!zvb_sound_fifo_full()) {
            zvb_peri_sound_sample_fifo = *data++;
            length--;
        }
    }

    /* Make sure all the samples have been played */
    zvb_sound_wait_empty();

    zvb_peri_sound_hold = s_mst_hold;
}
