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
#include "piano.h"

#define BPM         400
#define DURATION_MS (60000/(BPM))

uint16_t partition[PARTITION_MAX_LEN];
uint16_t partition_length;

/* Duration of a note in milliseconds */
uint16_t note_duration = 400;
sound_waveform_t note_waveform = 0;

static char* split_string(char* current, char** next)
{
    /* Make sure the current file is not invalid */
    if (*current == 0 || *current == ' ') {
        return NULL;
    }

    char* it = current;
    while (*it != ' ' && *it != 0) { it++; }
    *it = 0;
    /* Populate the next string to test */
    *next = ++it;

    return current;
}


int main(int argc, char** argv) {
    char* next;

    if (argc == 0) {
        printf("usage: ./play.bin notes.txt\n");
        return 1;
    }

    /* Get the name of the text file containing the notes */
    char* filename = split_string(argv[0], &next);
    if (filename == NULL) {
        printf("Invalid file name\n");
        return 1;
    }

    zos_err_t ret = parse_notes_file(filename);
    if (ret != 0) {
        printf("Error parsing file\n");
        return 1;
    }

    /* The file has been parsed and the table partition has been filled */
    zvb_sound_initialize(1);
    zvb_sound_set_voices(VOICE0, 0, note_waveform);
    /* Assign the channel to the left and right channels even if `initialize`
     * may have done it already. */
    zvb_sound_set_channels(VOICE0, VOICE0);
    zvb_sound_set_voices_vol(VOICE0, VOL_100);
    zvb_sound_set_volume(VOL_100);
    /* Let's play! */
    zvb_sound_set_hold(VOICE0, 0);

    for (uint16_t i = 0; i < partition_length; i++) {
        zvb_sound_set_voices(VOICE0, partition[i], note_waveform);
        msleep(note_duration - 3);
    }

    zvb_sound_set_volume(VOL_0);

    return 0;
}

