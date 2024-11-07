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
#include <zos_video.h>
#include <zvb_gfx.h>

#define FOREGROUND_LAYER    1
#define BACKGROUND_LAYER    0

/**
 * @brief Defining the graphic context as a static variable will make it easier for the compiler
 * to generate efficient code for the Z80 (compared to using the stack)
 */
static gfx_context vctx;


/**
 * @brief Palette for the graphics tiles. All the colors here are in RGB565 format, little-endian.
 */
const uint16_t palette[] = {
    /* Black/transparent color. Transparent will always be color0 for tiles on foreground layer */
    RGB888_TO_RGB565(0, 0, 0),
    /* Dark blue color */
    RGB888_TO_RGB565(0x00, 0x55, 0xa4),
    /* White color */
    RGB888_TO_RGB565(0xff, 0xff, 0xff),
    /* Red color */
    RGB888_TO_RGB565(0xef, 0x41, 0x35),
};


/**
 * @brief Call this function on error to reset the screen and exit the program
 */
void failure(void)
{
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    exit(1);
}


int main(int argc, char** argv)
{
    /* Disable the screen to prevent artifacts when setting up the scene */
    gfx_enable_screen(0);

    /* Initialize the graphics mode, let's use 320x240px resolution with 8-bit colors.
     * In this mode we have 20x15 visible tiles (16x16px each) on screen, and two layers:  foreground and background.
     * The foreground layer suppors transparency */
    gfx_error err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) failure();

    /* Load our palette starting at hardware palette index 0 */
    err = gfx_palette_load(&vctx, palette, sizeof(palette), 0);
    if (err) failure();

    /* Create four colored tiles, one for each color */
    for (uint8_t i = 0; i < 4; i++) {
        gfx_tileset_add_color_tile(&vctx, i, i);
    }

    /* Create a line of 20 transparent tiles, for the background,
     * Make this array static to optimize the code in speed and space.
     */
    static const uint8_t transparent[20] = { 0 };

    /* Create a line of 20 tiles that will be shown on all 15 lines: place 6 blue tiles, 8 white tiles, 6 red tiles.
     * The blue tile has index 1 (same as in the palette), white tile has index 2 and red tile has index 3.
     */
    static const uint8_t tilemap[20] = {
        1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3,
    };

    for (uint8_t line = 0; line <= 15; line++) {
        /* Put 20 transparent tiles on the foreground layer, at X = 0, Y = line */
        gfx_tilemap_load(&vctx, transparent, sizeof(transparent), FOREGROUND_LAYER, 0, line);
        /* Put the flag pattern on the background layer, at X = 0, Y = line */
        gfx_tilemap_load(&vctx, tilemap, sizeof(tilemap), BACKGROUND_LAYER, 0, line);
    }

    /* Finally, enable the screen */
    gfx_enable_screen(1);

    /* Sleep 5 seconds before exiting */
    msleep(5000);
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    return 0;
}


