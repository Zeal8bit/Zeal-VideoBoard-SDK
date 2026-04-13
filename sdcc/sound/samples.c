/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "sound_internal.h"

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
    uint8_t* data;

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
    zvb_peri_sound_hold = zvb_sound_master_hold & ~SAMPTAB;

    /* Play all the samples */
    data = (uint8_t*) samples;

    while (length > 0) {
        if (!zvb_sound_fifo_full()) {
            zvb_peri_sound_sample_fifo = *data++;
            length--;
        }
    }

    /* Make sure all the samples have been played */
    zvb_sound_wait_empty();

    zvb_peri_sound_hold = zvb_sound_master_hold;
}
