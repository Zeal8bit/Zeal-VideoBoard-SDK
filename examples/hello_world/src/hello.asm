;
; SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
;
; SPDX-License-Identifier: CC0-1.0

    INCLUDE "zvb_hardware_h.asm"

    ; Physical address of the RAM on Zela 8-bit Computer
    DEFC RAM_PHYS_ADDR = 0x80000

    ; Zeal 8-bit Computer's MMU related:
    ; It has 4 virtual pages of 16KB each
    DEFC PAGE0_VADDR = 0x0000
    DEFC PAGE1_VADDR = 0x4000
    DEFC PAGE2_VADDR = 0x8000
    DEFC PAGE3_VADDR = 0xC000

    DEFC MMU_PAGE_0 = 0xF0
    DEFC MMU_PAGE_1 = 0xF1
    DEFC MMU_PAGE_2 = 0xF2
    DEFC MMU_PAGE_3 = 0xF3

    ORG 0
    ; In this example we will use the following memory mapping:
    ; 0x0000:0x3FFF - Page 0 - Always mapped, this code
    ; 0x4000:0x7FFF - Page 1 - Unused
    ; 0x8000:0xBFFF - Page 2 - Unused
    ; 0xC000:0xFFFF - Page 3 - RAM, used for the stack

    ; Map some RAM in the last page (page 3)
    ld a, RAM_PHYS_ADDR >> 14
    out (MMU_PAGE_3), a
    ld sp, 0xffff

    ; The video board has a banked I/O controller. In other words, the I/O addresses
    ; 0xa0 to 0xaf can access several controllers, including the text monitor, the CRC32,
    ; the sound, etc...
    ; Let's map the text controller
    ld a, BANK_IO_TEXT_NUM
    out (IO_MAPPER_BANK), a

    ; We can now use all the IO_TEXT_* register defined in the header file
    ; Let's print a message, switch the colors for each character.
    ; The background color is the high nibble, the foreground color is the lower nibble
    ; We can use D for the color
    ld d, 0xf1
    ld hl, hello_msg
    ld b, hello_msg_end - hello_msg
print_loop:
    ; Set the color and change the color for the next character
    ld a, d
    out (IO_TEXT_COLOR), a
    inc d
    ; Print the next character from the string
    ld a, (hl)
    out (IO_TEXT_PRINT_CHAR), a
    inc hl
    djnz print_loop

wait:
    jr wait

hello_msg: DEFM "Hello, world!"
hello_msg_end:
