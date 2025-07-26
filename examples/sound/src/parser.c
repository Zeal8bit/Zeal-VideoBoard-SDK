/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include "piano.h"


/* The entries here represents the dividers (frequency) multiplied by 16
 * for the notes DO, RE, MI, FA, SOL, LA, SI, for the octave 5.
 * Going on lower octaves require to divide by 2 every time. */
const static uint16_t octave5[] = {
    SOUND_FREQ_TO_DIV(1047),
    SOUND_FREQ_TO_DIV(1109),
    SOUND_FREQ_TO_DIV(1175),
    SOUND_FREQ_TO_DIV(1245),
    SOUND_FREQ_TO_DIV(1319),
    SOUND_FREQ_TO_DIV(1397),
    SOUND_FREQ_TO_DIV(1480),
    SOUND_FREQ_TO_DIV(1568),
    SOUND_FREQ_TO_DIV(1661),
    SOUND_FREQ_TO_DIV(1760),
    SOUND_FREQ_TO_DIV(1865),
    SOUND_FREQ_TO_DIV(1976),
};

static zos_err_t parse_duration(const char* note) {
    extern uint16_t note_duration;
    uint16_t res = atoi(note);
    if (res < 50 || res > 1000) {
        return ERR_FAILURE;
    }
    note_duration = res;
    return ERR_SUCCESS;
}


static void parse_note(char* note)
{
    const char* backup = note;
    const char c1 = toupper(note[0]);
    const char c2 = toupper(note[1]);
    uint8_t index = 0;
    uint8_t hassharp = 1;

    /* Check if it's a silence */
    if (c1 == '-') {
        /* Ignore the next characters */
        partition[partition_length++] = 0;
        return;
    /* Check if it's a note duration directive */
    } else if (c1 == '=') {
        if (parse_duration(note+1) != ERR_SUCCESS) {
            printf("Invalid note duration, ignoring\n");
        }
        return;
    /* Check the type of wave */
    } else if (c1 == 'T') {
        if (c2 >= '0' && c2 <= '2') {
            extern sound_waveform_t note_waveform;
            note_waveform = (sound_waveform_t) (c2 - '0') | DUTY_CYCLE_50_0;
        } else {
            printf("Invalid waveform, ignoring\n");
        }
        return;
    } else if (c1 == 'D' && c2 == 'O') {
    } else if (c1 == 'R' && c2 == 'E') {
        index = 2;
    } else if (c1 == 'M' && c2 == 'I') {
        index = 4;
        hassharp = 0;
    } else if (c1 == 'F' && c2 == 'A') {
        index = 5;
    } else if (c1 == 'S' && c2 == 'O') {
        index = 7;
        /* Support both SO and SOL */
        if (note[2] == 'L'){
            note++;
        }
    } else if (c1 == 'L' && c2 == 'A') {
        index = 9;
    } else if (c1 == 'S' && c2 == 'I') {
        index = 11;
        hassharp = 0;
    } else {
        printf("Invalid note %s, ignoring\n", backup);
        return;
    }

    note += 2;
    char octave = *note++;
    if (octave >= '1' && octave <= '5') {
        octave -= '1';
    } else {
        printf("Invalid octave in %s, ignoring\n", backup);
        return;
    }

    if (*note == '#') {
        if (hassharp) {
            index++;
        } else {
            printf("Invalid note %s, ignoring\n", backup);
            return;
        }
    }

    /* Key the divider from the LUT */
    uint16_t div = octave5[index];

    octave = 4 - octave;
    while (octave) {
        div = div >> 1;
        octave--;
    }

    /* Store in the final partition */
    partition[partition_length++] = div;
}

static char buffer[1024];

static void parse_lines(uint16_t size)
{
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        while (*line == ' ') {
            line++;
        }
        /* Allow comments with character ; */
        if (*line != 0 && *line != ';') {
            parse_note(line);
        }

        line = strtok(NULL, "\n");
    }
}

zos_err_t parse_notes_file(const char* filename)
{
    zos_dev_t fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Could not open file %s\n", filename);
        return -fd;
    }

    while (1) {
        uint16_t size = 1024 - 1;
        int rd = read(fd, buffer, &size);
        if (rd != ERR_SUCCESS) {
            printf("Error reading file\n");
            return rd;
        } else if (size == 0) {
            break;
        }
        buffer[size] = 0;
        parse_lines(size);
    }

    return ERR_SUCCESS;
}
