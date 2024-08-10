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
 * The smaller the faster, must be > 0
 */
#define TITLE_BOUNCE_DELAY  3

/**
 * Bottom limit for the title
 */
#define TITLE_Y             64


extern gfx_context vctx;

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
    static int16_t velocity = 0;
    uint8_t delay = 0;

    while (1) {
        gfx_wait_vblank(&vctx);
        
        delay++;
        if (delay == TITLE_BOUNCE_DELAY) {
            delay = 0;
            velocity++;
            y += velocity;

            const uint16_t whole = y >> 8;
            if (whole > TITLE_Y) {
                y = (TITLE_Y << 8);
                if (velocity < 48) {
                    gfx_wait_end_vblank(&vctx);
                    return;
                }
                velocity = -velocity / 2;
            }

            gfx_sprite_set_y(&vctx, idx, whole);
        }
        gfx_wait_end_vblank(&vctx);
    }
}

void play_title(void) {
    /* Keep the trailing \0 to show the snake head */
    const char title[] = "SNAKE";
    const uint8_t x = 7;

    for (uint8_t i = 0; i < sizeof(title); i++) {
        bounce_letter(i, title[i], (x + i + 1) << 4);
    }

    /* Flip the snake */
    for (uint8_t i = 0; i < 4; i++) {
        gfx_sprite_flags flags = (i & 1) ? 0 : SPRITE_FLIP_Y;
        gfx_sprite_set_flags(&vctx, sizeof(title) - 1, flags);
        msleep(256);
    }

    msleep(1000);
    /* Hide the sprites */
    for (uint8_t i = 0; i < sizeof(title); i++) {
        gfx_sprite_set_y(&vctx, i, 0);
    }
}