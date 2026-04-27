; SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
;
; SPDX-License-Identifier: Apache-2.0

    .module zvb_crc
    .optsdcc -mz80

    .globl _zvb_crc_initialize
    .globl _zvb_crc_reset
    .globl _zvb_crc_update

_zvb_config_dev_idx   = 0x8e
_zvb_peri_crc_ctrl    = 0xa0
_zvb_peri_crc_data_in = 0xa1
_zvb_peri_crc_byte0   = 0xa4
_zvb_peri_crc_byte1   = 0xa5
_zvb_peri_crc_byte2   = 0xa6
_zvb_peri_crc_byte3   = 0xa7

    .area _TEXT


; void zvb_crc_initialize(uint8_t reset)
;
; Initialize and optionally reset the CRC accumulator.
; 
; Parameters:
;   A - Reset the accumulator if not 0
_zvb_crc_initialize::
    ; Preserve reset while mapping the peripheral because mapping uses A.
    ld c, a

    ; Map peripheral window 0xa0..0xaf to the CRC device.
    ld a, #0x02
    out (_zvb_config_dev_idx), a

    ; If reset == 0, initialization is done.
    ld a, c
    or a, a
    ret z

    ; Reset CRC accumulator.
    ld a, #0x01
    out (_zvb_peri_crc_ctrl), a
    ret


; void zvb_crc_reset(void)
; Reset the CRC accumulator.
; Parameters:
;   None
; Returns:
;   None
_zvb_crc_reset::
    ; Map peripheral window 0xa0..0xaf to the CRC device.
    ld a, #0x02
    out (_zvb_config_dev_idx), a
    ld a, #0x01
    out (_zvb_peri_crc_ctrl), a
    ret


; uint32_t zvb_crc_update(uint8_t* buffer, uint16_t size) __sdcccall(1)
;
; The CRC hardware consumes bytes written to _zvb_peri_crc_data_in. OTIR writes
; B bytes from HL to port C and increments HL. With B = 0, OTIR writes 256 bytes.
; The loop count in A is ceil(size / 256), while initial B is size & 0xff.
;
; Parameters:
;   HL - Buffer to calculate the CRC of
;   DE - Size of the buffer
;
; Returns:
;   HLDE = 32-bit CRC value (E is LSB, H is MSB)
_zvb_crc_update::
    ; Map peripheral window 0xa0..0xaf to the CRC device.
    ld a, #0x02
    out (_zvb_config_dev_idx), a

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
    ld c, # _zvb_peri_crc_ctrl + 0x1
zvb_crc_update_loop:
    otir
    ; Decrement the number of loops
    dec a
    ; On non-zero, continue the loop (B is already 0!)
    jp nz, zvb_crc_update_loop
    ; On carry, end of the loop (A was 0 before the sub instruction)
zvb_crc_update_ret:
    ; Extract the resulting 32-bit value in HLDE
    in a, (_zvb_peri_crc_ctrl + 0x4)
    ld e, a
    in a, (_zvb_peri_crc_ctrl + 0x5)
    ld d, a
    in a, (_zvb_peri_crc_ctrl + 0x6)
    ld l, a
    in a, (_zvb_peri_crc_ctrl + 0x7)
    ld h, a
    ret
