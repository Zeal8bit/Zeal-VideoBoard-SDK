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
    ; 0x4000:0x7FFF - Page 1 - Banked ROM, used to copy the parrot tileset (~58KB)
    ; 0x8000:0xBFFF - Page 2 - Banked VRAM, used for tileset, tilemap, etc...
    ; 0xC000:0xFFFF - Page 3 - RAM, used for the stack

    ; Map some RAM in the last page (page 3)
    ld a, RAM_PHYS_ADDR >> 14
    out (MMU_PAGE_3), a
    ld sp, 0xffff

    ; To prevent artifacts/garbage on screen, we can turn it off during the setup.
    ; It is not mandatory to do so if we want to write to VRAM, it's more a matter of taste
    ; and a matter of how long the setup below will take. If only a single byte was modified in
    ; the tilemap, turning the screen off and on would be useless.
    xor a
    out (IO_CTRL_STATUS_REG), a

    ; Switch to 8-bit GFX 320x240 mode
    ld a, VID_MODE_GFX_320_8BIT
    out (IO_CTRL_VID_MODE), a

    ; Map the VRAM tileset (first 16KB) to the page 2
    ld a, VID_MEM_TILESET_ADDR >> 14
    ld d, a ; Keep the VRAM page in D
    out (MMU_PAGE_2), a
    ; And the parrot tileset first 16KB into page 1 (physical address is 0x4000)
    ld a, 0x4000 >> 14
    ld e, a ; Keep the ROM page in E
    out (MMU_PAGE_1), a

    ; Copy these 16KB from ROM to VRAM, do this 3 times (3 * 16KB = 48KB)
    ld h, 3
    ld bc, 16*1024
_copy_tileset:
    call copy_page1_to_page2
    ; Map the next VRAM page
    inc d
    ld a, d
    out (MMU_PAGE_2), a
    ; Similarly for the ROM page
    inc e
    ld a, e
    out (MMU_PAGE_1), a
    dec h
    jr nz, _copy_tileset
    ; The remaining data are not 16KB big, so we need to adjust the length
    ld bc, _parrot_tileset_end - _parrot_tileset - 48*1024
    call copy_page1_to_page2
    ; Create a transparent tile at index 255, so that we can fill layer1 (foreground) with it
    ld hl, PAGE3_VADDR - 256
    ld bc, 256  ; each tile is 256 bytes big
    xor a       ; transparent color is 0
    call memset

    ; The tileset has been copied, now let's map the first 16KB of VRAM which contains the tilemaps and the palette
    ld a, VID_MEM_PHYS_ADDR_START >> 14
    out (MMU_PAGE_2), a
    ; Copy the parrot color palette (in page 0 of the virtual memory)
    ld hl, _parrot_palette
    ld de, PAGE2_VADDR + VID_MEM_PALETTE_OFFSET
    ld bc, _parrot_palette_end - _parrot_palette
    ldir

    ; Finally, we can copy the parrot tilemap. Fill the layer1 with transparent tile (255).
    ld hl, PAGE2_VADDR + VID_MEM_LAYER1_OFFSET
    ld a, 255
    ld bc, 3200 ; 40 lines of 80 tiles (because most of the screen is not visible now, it depends on the scrolling values)
    call memset

    ; Note that we cannot simply use an ldir instruction here, this is because each line is in fact made out of 80 tiles (that
    ; can be shown when scrolling) but our tilemap data/file considers that there are 20 tiles per line, so we need to
    ; implement an algorithm that is a bit more tricky.
    ld hl, _parrot_tilemap
    ld de, PAGE2_VADDR
    ld c, 15    ; 15 lines of 20 tiles to copy
_copy_tilemap:
    call copy_line
    dec c
    jr nz, _copy_tilemap

    ; The parrot is ready, turn on the screen!
    ld a, 0x80
    out (IO_CTRL_STATUS_REG), a

    ; Wait until the computer is powered off
wait:
    jr wait


    ; Parameters:
    ;   BC - Number of bytes to copy (> 0)
    ; Alters:
    ;   None
copy_page1_to_page2:
    push hl
    push bc
    push de
    ld hl, PAGE1_VADDR
    ld de, PAGE2_VADDR
    ldir
    pop de
    pop bc
    pop hl
    ret

    ; Copy the byte stored in register A to the memory location HL, BC times
memset:
    ld e, a
_memset_loop:
    ld (hl), e
    inc hl
    dec bc
    ld a, b
    or c
    jp nz, _memset_loop
    ret


copy_line:
    push bc
    ld bc, 20
    ldir
    ; Add 60 to the destination (DE) so that it points to the next tile line on screen
    ld bc, 60
    ex de, hl
    add hl, bc
    ex de, hl
    pop bc
    ret


_parrot_palette:
    INCBIN "assets/parrot.ztp"
_parrot_palette_end:

_parrot_tilemap:
    INCBIN "assets/parrot.ztm"
_parrot_tilemap_end:

    ; To simplify the code, make sure the tileset starts at address 16KB in ROM
    SECTION TILESET
    ORG 16*1024
_parrot_tileset:
    INCBIN "assets/parrot.zts"
_parrot_tileset_end: