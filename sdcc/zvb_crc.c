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
__asm
    ; Buffer in HL, size in DE
    ld a, d
    or e
    jr z, zvb_crc_update_ret
    ; Use OTIR instructions to go faster, divide the size in number of 256-byte loops
    ; Round it up since the loop below will stop when A is 0
    ld a, d
    ld b, e
    ; If B is not 0, A = D+1, else A = D
    dec b
    inc b
    jr z, zvb_crc_update_no_add
    inc a
zvb_crc_update_no_add:
    ; Set C to the DATA IN register
    ld c, # ZVB_PERI_BASE + 0x1
zvb_crc_update_loop:
    otir
    ; Decrement the number of loops
    dec a
    ; On non-zero, continue the loop (B is already 0!)
    jp nz, zvb_crc_update_loop
    ; On carry, end of the loop (A was 0 before the sub instruction)
zvb_crc_update_ret:
    ; Extract the resulting 32-bit value in HLDE
    in a, (ZVB_PERI_BASE + 0x4)
    ld e, a
    in a, (ZVB_PERI_BASE + 0x5)
    ld d, a
    in a, (ZVB_PERI_BASE + 0x6)
    ld l, a
    in a, (ZVB_PERI_BASE + 0x7)
    ld h, a
    ret
__endasm;
    // Unreachable but prevents a warning
    return 0;
}
