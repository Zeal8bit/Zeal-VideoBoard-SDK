/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 * SPDX-FileContributor: Originally authored by David Higgins <https://github.com/zoul0813>
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
#include <zvb_hardware.h>
#include "tilemap.h"

#define TILE_TRANSPARENT    0x1F
#define BACKGROUND_TILE     0x11
#define PLAYER_TILE         0x01
#define WIDTH               20
#define HEIGHT              15



gfx_context vctx;
gfx_sprite player;

int main(void) {
    init();

    uint8_t frames = 0;
    while (1) {
        gfx_wait_vblank(&vctx);
        frames++;
        if(frames == 12) {
            draw();
            frames = 0;
        }
        gfx_wait_end_vblank(&vctx);
    }
    // return 0; // unreachable
}

void init(void) {
    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    gfx_error err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    // Load the palette
    extern uint8_t _cave_palette_end;
    extern uint8_t _cave_palette_start;
    const size_t palette_size = &_cave_palette_end - &_cave_palette_start;
    err = gfx_palette_load(&vctx, &_cave_palette_start, palette_size, 0);
    if (err) exit(1);

    // Load the tiles
    extern uint8_t _cave_tileset_end;
    extern uint8_t _cave_tileset_start;
    const size_t sprite_size = &_cave_tileset_end - &_cave_tileset_start;
    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
    };
    err = gfx_tileset_load(&vctx, &_cave_tileset_start, sprite_size, &options);
    if (err) exit(1);

    // Draw the tilemap
    load_tilemap();

    // Setup the player sprite
    player.flags = SPRITE_NONE;
    gfx_sprite_set_tile(&vctx, 0, PLAYER_TILE);
    player.tile = PLAYER_TILE;

    gfx_enable_screen(1);
}

void load_tilemap(void) {
    uint8_t line[WIDTH];

    // Load the tilemap
    extern uint8_t _cave_tilemap_start;
    for (uint16_t i = 0; i < HEIGHT; i++) {
        uint16_t offset = i * WIDTH;
        memcpy(&line, &_cave_tilemap_start + offset, WIDTH);
        gfx_tilemap_load(&vctx, line, WIDTH, 0, 0, i);
    }
}

/**
 * Simple Edge Bounce animation
*/
void draw(void) {
    static uint16_t x = 16;
    static uint16_t y = 16;
    static int8_t xd = 1;
    static int8_t yd = 1;

    x += xd;
    y += yd;

    if(x >= 320) { // 320 - 16
        xd = -1;
    }
    if(x <= 16) {
        xd = 1;
    }

    if(y >= 240) { // 240 - (16 * 2)
        yd = -1;
    }
    if(y <= 16) {
        yd = 1;
    }

    player.x = x;
    player.y = y;

    if(xd < 0) {
        player.flags = SPRITE_NONE;
    } else {
        player.flags = SPRITE_FLIP_X;
    }

    uint8_t err = gfx_sprite_render(&vctx, 0, &player);
    if(err != 0) {
        printf("graphics error: %d", err);
        exit(1);
    }
}

void _cave_palette(void) {
    __asm__(
    "__cave_palette_start:\n"
    "    .incbin \"assets/tilemap.ztp\"\n"
    "__cave_palette_end:\n"
    );
}

void _cave_tileset(void) {
    __asm__(
    "__cave_tileset_start:\n"
    "    .incbin \"assets/tilemap.zts\"\n"
    "__cave_tileset_end:\n"
    );
}

void _cave_tilemap(void) {
    __asm__(
    "__cave_tilemap_start:\n"
    "    .incbin \"assets/tilemap.ztm\"\n"
    "__cave_tilemap_end:\n"
    );
}
