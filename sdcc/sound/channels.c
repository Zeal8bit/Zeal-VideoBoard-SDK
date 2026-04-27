/**
 * SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sound_internal.h"

void zvb_sound_set_channels(sound_voice_t left_voices, sound_voice_t right_voices)
{
    zvb_sound_map();
    zvb_peri_sound_left_channel = left_voices;
    zvb_peri_sound_right_channel = right_voices;
}
