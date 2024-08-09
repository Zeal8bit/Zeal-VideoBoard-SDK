/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include <zos_time.h>
#include <zvb_gfx.h>
#include "sprites.h"

static void init_game(void);
static void draw(void);
static void update(void);

gfx_context vctx;
gfx_sprite sprite;

int main(void) {
    init_game();

    uint8_t frames = 0;
    while (1) {
        frames++;
        if(frames % 120 == 0) {
            update();
            draw();
            frames = 1;
        }
    }
    // return 0; // unreachable
}

static void init_game(void) {
    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    gfx_error err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    extern uint8_t _char_palette_end;
    extern uint8_t _char_palette_start;
    const size_t palette_size = &_char_palette_end - &_char_palette_start;

    err = gfx_palette_load(&vctx, &_char_palette_start, palette_size, 0);
    if (err) exit(1);

    extern uint8_t _char_sprite_end;
    extern uint8_t _char_sprite_start;
    const size_t sprite_size = &_char_sprite_end - &_char_sprite_start;
    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte = 0x4001
    };
    err = gfx_tileset_load(&vctx, &_char_sprite_start, sprite_size, &options);
    if (err) exit(1);

    gfx_sprite_set_tile(&vctx, 0, 0);
    gfx_sprite_set_tile(&vctx, 1, 1);

    gfx_enable_screen(1);
}

static void draw(void) {
    static uint16_t x = 16;
    static uint16_t y = 0;
    static int8_t xd = 1;
    static int8_t yd = 1;
    /* Wait for v-blank */
    gfx_wait_vblank(&vctx);


    x += xd;
    y += yd;

    if(x > 320) {
        xd = -1;
    }
    if(x <= 16) {
        xd = 1;
    }

    if(y > 288) { // 240 - (16 * 2)
        yd = -1;
    }
    if(y <= 16) {
        yd = 1;
    }

    gfx_sprite sprite = {
        .x = x,
        .y = y,
        .tile = 0,
        .flags = SPRITE_NONE,
    };
    gfx_sprite_render(&vctx, 0, &sprite);
    sprite.y += 16;
    sprite.tile = 1;
    gfx_sprite_render(&vctx, 0, &sprite);
}

static void update(void) {
    // move sprites
}

/**
 * @brief Workaround to include a binary file in the program
 */
void _char_palette() {
    __asm
__char_palette_start:
    .incbin "assets/chars.ztp"
__char_palette_end:
    __endasm;
}

/**
 * @brief Workaround to include a binary file in the program
 */
void _char_sprite() {
    __asm
__char_sprite_start:
    .incbin "assets/chars.zts"
__char_sprite_end:
    __endasm;
}
