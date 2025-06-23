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

static inline void zvb_sound_map(void)
{
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}

void zvb_sound_initialize(uint8_t reset)
{
    zvb_sound_map();
    if (reset) {
        zvb_peri_sound_select = VOICE0 | VOICE1 | VOICE2 | VOICE3;

        zvb_peri_sound_master_vol = ZVB_PERI_SOUND_VOL_DISABLE;
        zvb_peri_sound_volume = ZVB_PERI_SOUND_VOL_100;
        zvb_peri_sound_volume_left = VOICE0 | VOICE1 | VOICE2 | VOICE3;
        zvb_peri_sound_volume_right = VOICE0 | VOICE1 | VOICE2 | VOICE3;

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


static void zvb_sound_wait_samples(void)
{
    while ((zvb_peri_sound_sample_conf & 0x80) == 0) {
    }
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

    /* Hold all the voices but the current one */
    zvb_peri_sound_hold = 0xff;

    /* Set the master volume */
    zvb_peri_sound_master_vol = VOL_100;

    /* Play all the samples */
    uint16_t remaining = length;
    uint8_t* data = (uint8_t*) samples;

    /* If played in the emulator, no need to split the samples into smaller chunks */
    if ((zvb_peri_sound_sample_conf >> 6) & 1) {
        zvb_sound_wait_samples();
        while (remaining--) {
            zvb_peri_sound_sample_fifo = *data++;
        }
        zvb_peri_sound_hold = ~SAMPTAB;
    } else {
        while (remaining > 0) {
            /* Calculate the minimum between the FIFO length and the remaining bytes */
            uint16_t min = remaining < SOUND_SAMPLE_TABLE_SIZE ? remaining : SOUND_SAMPLE_TABLE_SIZE;

            /* Wait for the voice to be ready (i.e. finish outputting previous samples) */
            zvb_sound_wait_samples();

            /* Subtract the remaining length before we alter `min` variable */
            remaining -= min;
            while (min--) {
                zvb_peri_sound_sample_fifo = *data++;
            }

            /* Unhold the sample table voice */
            zvb_peri_sound_hold = ~SAMPTAB;
        }
    }
}
