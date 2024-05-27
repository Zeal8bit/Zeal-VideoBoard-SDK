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

    DEFC ANIM_START_IDX = 0xC000
    DEFC POS_X_IDX      = 0xC001
    DEFC DIRECTION_IDX  = 0xC002
    DEFC ADDER_IDX      = 0xC003
    DEFC FLIP_ATTR_X    = 0xC004
    DEFC FRAME_COUNT    = 0xD000

    DEFC MAX_SPRITES = 128

    ORG 0

    ; Map some RAM in the last page (page 3)
    ld a, RAM_PHYS_ADDR >> 14
    out (MMU_PAGE_3), a
    ld sp, 0xffff

    call wait_for_vblank

    ; Switch to 8-bit GFX 320x240 mode
    ld a, VID_MODE_GFX_320_8BIT
    out (IO_CTRL_VID_MODE), a

    ; Map the VRAM tileset (first 16KB) to the page 1
    ld a, VID_MEM_PHYS_ADDR_START >> 14
    out (MMU_PAGE_1), a

    ; Write the palette to the FPGA
    ld hl, _palette
    ld de, PAGE1_VADDR + VID_MEM_PALETTE_OFFSET
    ld bc, _palette_end - _palette
    ldir

    ; Map first 16KB of the VRAM tileset at virtual address 0x8000
    ld a, VID_MEM_TILESET_ADDR >> 14
    out (MMU_PAGE_2), a
    ; Create a black tile
    ld hl, PAGE2_VADDR
    ld a, 1
    ld bc, 256
    call memset

    ; Copy the tileset to the VRAM tileset
    ex de, hl
    ld hl, _tileset
    ld bc, _tileset_end - _tileset
    ldir

    ; Fill the foreground tileset to with tiles 0 (black). Therefore there is no need
    ; to fill the background tilemap, it won't be visible anyway.
    ld hl, PAGE1_VADDR
    ld bc, 3200 ; 80*40 tiles (including the ones not visible)
    xor a
    call memset

    ; Initialization phase, draw the sprites, 8 per line
    ld b, 0 ; Line
    ld de, PAGE1_VADDR + VID_MEM_SPRITES_OFFSET
_next_line:
    ld c, 0 ; Column
_next_sprite:
    ; Y coordinates: B * 16 + 16
    ld l, b
    ld h, 0
    call hl_mul_16_add_16
    ; [DE] = HL
    ld a, l
    ld (de), a
    inc de
    ld a, h
    ld (de), a
    inc de
    ; X coordinates: C * 16 + 16
    ld l, c
    ld h, 0
    call hl_mul_16_add_16
    ld a, l
    ld (de), a
    inc de
    ld a, h
    ld (de), a
    inc de
    ; Tile number (1 + Y[0])
    ld a, b
    and 1
    add 1
    ld (de), a
    inc de
    xor a
    ld (de), a
    inc de
    ; Empty word
    inc de
    inc de
    ; Next column
    inc c
    ; If C is 10, go to next line
    ld a, c
    sub 8
    jr nz, _next_sprite
    ld c, a
    inc b
    ; If we reached last line, stop
    ld a, b
    cp MAX_SPRITES / 8
    jr nz, _next_line

    ; Set the initial tile to 0
    xor a
    ld (ANIM_START_IDX), a
    ld (POS_X_IDX), a
    ld (DIRECTION_IDX), a
    ld (FLIP_ATTR_X), a
    inc a
    ld (ADDER_IDX), a

    ld b, 180
wait_loop:
    call wait_end_vblank
    call wait_for_vblank
    djnz wait_loop


    ; Wait for the next frame
_process_frame:
    call next_frame
    ld hl, FRAME_COUNT
    inc (hl)
    ; Animate every 4 frames the X position of the sprites are updated
    ld a, (hl)
    push af
    ; A = 1 <=> animation ended
    call update_position
    or a
    jr nz, animation_end
    pop af
    and 0xf
    push af
    call z, update_tiles
    pop af
    jr nz, _process_frame
    ld (hl), 0
    jr _process_frame

animation_end:
    pop af  ; clean the stack
    ; Reset the sprites tiles
    ; ld a, 6
    ; call update_tiles
    ; ld b, 60
    ; call wait
    ; Reset the X position
    xor a
    ld (POS_X_IDX), a
    ; Change the direction
    ld hl, DIRECTION_IDX
    ld a, (hl)
    xor 1
    ld (hl), a
    ; Flip X bit
    ld hl, FLIP_ATTR_X
    ld a, (hl)
    xor 0b00001000
    ld (hl), a
    jp _process_frame

wait:
    call wait_end_vblank
    call wait_for_vblank
    djnz wait

    DEFC FINAL_POSITION = 160 + 32

update_position:
    ld hl, POS_X_IDX
    ld a, (hl)
    cp FINAL_POSITION
    jr z, update_position_end
    inc a
    ld (hl), a
    ld c, a
    ; Need to add A to all the sprites' X coordinates
    ld hl, PAGE1_VADDR + VID_MEM_SPRITES_OFFSET + 2
    ld b, MAX_SPRITES
_update_position_loop:
    ld a, MAX_SPRITES
    sub b
    ; Get inital X coordinates of the sprite
    and 7
    ; Set it to X * 16 + C
    ex de, hl
    ld l, a
    ld h, 0
    call hl_mul_16_add_16
    ; HL += C if DIRECTION_IDX is 0, else HL += (FINAL_POSITION - C)
    ld a, (DIRECTION_IDX)
    or a
    ld a, c
    jr z, _position_to_right
    ld a, FINAL_POSITION
    sub c
_position_to_right:
    add l
    ld l, a
    adc h
    sub l
    ld h, a
    ex de, hl
    ld (hl), e
    inc hl
    ld (hl), d
    inc hl
    ; HL += 6
    ld a, 6
    add l
    ld l, a
    adc h
    sub l
    ld h, a
    djnz _update_position_loop
    xor a
    ret
update_position_end:
    ld a, 1
    ret

    ; Browse all the sprites and update the tiles to be (X+2) & 7
    ; Because we have 8 tiles in total
update_tiles:
    ld hl, PAGE1_VADDR + VID_MEM_SPRITES_OFFSET + 4
    ld b, MAX_SPRITES
    ; Calculate the next tile to show
    ld a, (ANIM_START_IDX)
    add 2
    and 7
    ld (ANIM_START_IDX), a
    ; Get the tile address in DE
    ld de, table
    add e
    ld e, a
_update_tiles_loop:
    ; If the sprite is on an odd line, we have to add 1, again
    ld a, MAX_SPRITES
    sub b
    ; divide by 8
    rrca
    rrca
    rrca
    and 1
    ld c, a
    ld a, (de)
    add c
    ld (hl), a
    inc hl
    ; Flip X?
    ld a, (FLIP_ATTR_X)
    ld (hl), a
    ; HL + 7
    ld a, 7
    add l
    ld l, a
    adc h
    sub l
    ld h, a
    djnz _update_tiles_loop
    ret


    ; Tileset has repetition...
    ALIGN 8
table: DEFM 1, 2, 3, 4, 1, 2, 3, 5

hl_mul_16_add_16:
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    ; HL += 16
    ld a, 0x10
    add l
    ld l, a
    adc h
    sub l
    ld h, a
    ret


next_frame:
    call wait_end_vblank
    jp wait_for_vblank

    ; Wait for the FPGA to be initialized, read the status register and wait for VBlank
wait_for_vblank:
    in a, (IO_CTRL_STATUS_REG)
    and 2
    jr z, wait_for_vblank
    ret

wait_end_vblank:
    in a, (IO_CTRL_STATUS_REG)
    and 2
    jr nz, wait_end_vblank
    ret


setline:
    ld b, 20
setline_loop:
    ld (hl), a
    inc hl
    djnz setline_loop
    ; Add 60 to HL so that it points to the next line of screen!
    push bc
    ld bc, 60
    add hl, bc
    pop bc
    ret


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


_palette:
    INCBIN "chars.ztp"
_palette_end:
_tileset:
    INCBIN "chars.zts"
_tileset_end: