/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "zvb_crc.h"


void zvb_crc_initialize(uint8_t reset)
{
    zvb_map_peripheral(ZVB_PERI_CRC_IDX);
    if (reset) {
        zvb_peri_crc_ctrl = BIT(IO_CRC32_CTRL_RESET_BIT);
    }
}


void zvb_crc_reset(void)
{
    zvb_peri_crc_ctrl = BIT(IO_CRC32_CTRL_RESET_BIT);
}


uint32_t zvb_crc_update(uint8_t *buffer, uint16_t size)  __sdcccall(1)
{
    (void) buffer;
    (void) size;
    zvb_map_peripheral(ZVB_PERI_CRC_IDX);
    __asm__(
        "    ; Buffer in HL, size in DE\n"
        "    ld a, d\n"
        "    or e\n"
        "    jr z, zvb_crc_update_ret\n"
        "    ; Use OTIR instructions to go faster, divide the size in number of 256-byte loops\n"
        "    ; Round it up since the loop below will stop when A is 0\n"
        "    ld a, d\n"
        "    ld b, e\n"
        "    ; If B is not 0, A = D+1, else A = D\n"
        "    dec b\n"
        "    inc b\n"
        "    jr z, zvb_crc_update_no_add\n"
        "    inc a\n"
        "zvb_crc_update_no_add:\n"
        "    ; Set C to the DATA IN register\n"
        "    ld c, # ZVB_PERI_BASE + 0x1\n"
        "zvb_crc_update_loop:\n"
        "    otir\n"
        "    ; Decrement the number of loops\n"
        "    dec a\n"
        "    ; On non-zero, continue the loop (B is already 0!)\n"
        "    jp nz, zvb_crc_update_loop\n"
        "    ; On carry, end of the loop (A was 0 before the sub instruction)\n"
        "zvb_crc_update_ret:\n"
        "    ; Extract the resulting 32-bit value in HLDE\n"
        "    in a, (ZVB_PERI_BASE + 0x4)\n"
        "    ld e, a\n"
        "    in a, (ZVB_PERI_BASE + 0x5)\n"
        "    ld d, a\n"
        "    in a, (ZVB_PERI_BASE + 0x6)\n"
        "    ld l, a\n"
        "    in a, (ZVB_PERI_BASE + 0x7)\n"
        "    ld h, a\n"
        "    ret\n"
    );
    // Unreachable but prevents a warning
    return 0;
}
