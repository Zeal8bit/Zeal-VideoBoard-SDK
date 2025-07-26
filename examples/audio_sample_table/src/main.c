/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zvb_sound.h>

/**
 * These symbols are in fact defined in the inline assembly code in `_sample_raw` function.
 * This is a workaround to get raw binary files included in C code.
 */
extern uint8_t _sample_end;
extern uint8_t _sample_start;

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;
    /* Initialize the sound controller and reset it */
    zvb_sound_initialize(1);
    /* Assign the sample table to both channels */
    zvb_sound_set_channels(SAMPTAB, SAMPTAB);
    zvb_sound_set_volume(VOL_100);

    /* The sample in the `hello.bin` audio file are signed 16 bit values, it
     * must be played at a sample rate of ~11022Hz, so the divider is:
     * [44091 / 11022] - 1 = 3 */
    sound_samples_conf_t config = {
        .mode = SAMPLE_SINT16,
        .divider = 3
    };

    /* SDCC doesn't let us easily include a binary file, so use the
     * `_sample_raw` function for that */
    const size_t sample_size = &_sample_end - &_sample_start;
    zvb_sound_play_samples(&config, &_sample_start, sample_size);
    zvb_sound_set_volume(VOL_0);
    return 0;
}

void _sample_raw() {
    __asm
__sample_start:
    .incbin "hello_raw_audio.bin"
__sample_end:
    __endasm;
}
