/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zvb_crc.h>

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


static uint32_t calculate_file_crc32(const char* name)
{
    zos_err_t err;
    uint16_t size;
    /* Use static memory to avoid stack allocations */
    static uint8_t buf[1024];

    zos_dev_t fd = open(name, O_RDONLY);
    if (fd < 0) {
        printf("Error opening file %s\n", name);
        exit(1);
    }

    zvb_crc_initialize(1);
    while (1) {
        size = sizeof(buf);
        err = read(fd, buf, &size);
        if (err != ERR_SUCCESS) {
            printf("Error reading file %s\n", name);
            exit(1);
        }
        if (size == 0) {
            break;
        }
        zvb_crc_update(buf, size);
    }
    return zvb_crc_update(NULL, 0);
}


int main(int argc, char** argv) {
    /* On Zeal 8-bit OS, the argc is either 0 or 1, the strings are not split */
    if (argc != 1) {
        printf("usage: crc32.bin <file1> [<file2>] ... [<filen>]\n");
        return 1;
    }

    /* Look for the next file */
    char* next = NULL;
    char* name = split_string(argv[0], &next);
    while (name) {
        const uint32_t crc32 = calculate_file_crc32(name);
        printf("%08lx    %s\n", crc32, name);
        name = split_string(next, &next);
    }

    return 0;
}

