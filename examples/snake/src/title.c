/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zvb_gfx.h>

/**
 * The higher the faster, must be > 0
 */
#define BOUNCE_VELOCITY  36

/**
 * Bottom limit for the title
 */
#define TITLE_Y             64


extern gfx_context vctx;
unsigned long _title_size = 0;

static void bounce_letter(uint8_t idx, uint8_t letter, uint16_t x)
{
    /* Use the sprite 0 for the letter S */
    static gfx_sprite spr = { 0 };
    spr.x = x;
    spr.tile = letter;

    /* Wait for the next v-blank */
    gfx_wait_vblank(&vctx);
    gfx_sprite_render(&vctx, idx, &spr);
    gfx_wait_end_vblank(&vctx);

    uint16_t y = 0;
    static int16_t velocity;
    uint8_t delay = 0;
    velocity = 0;

    while (1) {
        gfx_wait_vblank(&vctx);

        velocity+= BOUNCE_VELOCITY;
        y += velocity;

        uint16_t whole = y >> 8;
        if(whole > TITLE_Y) {
            y = (TITLE_Y << 8);
            if (velocity <= 10 * BOUNCE_VELOCITY) {
                gfx_wait_end_vblank(&vctx);
                return;
            }
            velocity = -velocity / 2;
            whole = TITLE_Y;
        }

        gfx_sprite_set_y(&vctx, idx, whole);
        gfx_wait_end_vblank(&vctx);
    }
}

static uint8_t flip_head = 0;
void title_flip_head(void) {
    flip_head ^= 1;
    gfx_sprite_flags flags = flip_head ? 0 : SPRITE_FLIP_Y;
    gfx_sprite_set_flags(&vctx, _title_size - 1, flags);
}


void title_play(void) {
    /* Keep the trailing \0 to show the snake head */
    const char title[] = "SNAKE";
    _title_size = sizeof(title);
    const uint8_t x = 7;

    for (uint8_t i = 0; i < _title_size; i++) {
        bounce_letter(i, title[i], (x + i + 1) << 4);
        gfx_sprite_set_y(&vctx, i, TITLE_Y);
    }
}

void title_hide(void) {
    /* Hide the sprites */
    for (uint8_t i = 0; i < _title_size; i++) {
        gfx_sprite_set_y(&vctx, i, 0);
    }
}