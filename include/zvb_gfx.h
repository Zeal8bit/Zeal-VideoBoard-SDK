/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_hardware.h"

typedef uint8_t gfx_error;

#define GFX_SUCCESS     0
#define GFX_FAILURE     1
#define GFX_INVALID_ARG 2


#define TILESET_COMP_NONE   0
#define TILESET_COMP_1BIT   1
#define TILESET_COMP_4BIT   2
#define TILESET_COMP_RLE    16


/**
 * @brief Graphics context initialize by the init function
 */
typedef struct {
    uint8_t video_mode;
    uint8_t bpp;
    /* Index of the alpha color */
    uint8_t alpha_idx;

    /* Mapping related */
    uint8_t virtual_page;
    uint8_t backup_page;
} gfx_context;


typedef struct {
    uint8_t compression;
    /* Index of the byte to start loading to */
    uint16_t from_byte;
    /* Offset in the palette. This offset will be added to all the bytes (apart from 0/transparent)
     * Useful when several sub-palettes are stored in the main palette */
    uint8_t pal_offset;
    /* Enable opacity for compressed tilesets. A nibble 0 will ignore pal_offset */
    uint8_t opacity;
} gfx_tileset_options;


/**
 * @brief Initialize graphics with the given mode and virtual page to use
 *
 * @param mode Mode choosen from `ZVB_CTRL_VID_MODE_*` defines
 * @param out Initialized graphics context handler
 */
gfx_error gfx_initialize(uint8_t mode, gfx_context* out);


/**
 * @brief Enable or disable the screen
 *
 * @param ena When 0, disables the screen, else enables it
 */
void gfx_enable_screen(uint8_t ena);


/**
 * @brief Load a (part of) color palette in video memory.
 *
 * @note The palette is made out of 256 RGB565 colors.
 *
 * @param context Graphics context, must be initialized
 * @param palette Address of the colors to load in video memory
 * @param size Size of the palette to load, in bytes (and not color count)
 * @param from Color index to start loading from. Convenient to replace a part of the palette.
 */
gfx_error gfx_palette_load(gfx_context* ctx, void* palette, uint16_t size, uint8_t from);


/**
 * @brief Load a tileset into video memory
 *
 * @note The tileset can be 4-bit or 1-bit compressed. This can be used to reduce the size of the tileset
 *       when the current graphic mode is 8bpp.
 *
 * @param context Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure to give more info about the tileset. Can be NULL for raw tileset.
 */
gfx_error gfx_tileset_load(gfx_context* ctx, void* tileset, uint16_t size, const gfx_tileset_options* options);


/**
 * @brief Create a tile with a given color
 *
 * @param context Graphics context, must be initialized
 * @param index Index of the new tile (0-511)
 * @param color Color to fill the tile with
 */
gfx_error gfx_tileset_add_color_tile(gfx_context* ctx, uint16_t index, uint8_t color);


/**
 * @brief Load a line of tiles on the tilemap
 *
 * @note Regardless of the video mode, there are 80 tiles per line and 40 lines, so the screen coordinated
 *       can go up to (79, 39). To show them all, it is possible to use scrolling.
 *
 * @param context Graphics context, must be initialized
 * @param tiles Array of bytes representing the tiles to render (at most 80)
 * @param size Size of the tiles array, in bytes
 * @param layer Layer (0 or 1) to load the tiles to. Ignored in 4bpp mode.
 * @param x X coordinate to start loading from, in number of tiles (not pixel)
 * @param y Y coordinate to start loading from, in number of tiles (not pixel)
 */
gfx_error gfx_tilemap_load(gfx_context* ctx, void* tiles, uint8_t size, uint8_t layer, uint8_t x, uint8_t y);


/**
 * @brief Place a single tile on the given tilemap layer
 *
 * @param context Graphics context, must be initialized
 * @param tile Index of the tile to show on screen
 * @param layer Layer to show the tile on, must be 0 or 1
 * @param x X coordinate to show the tile at, in number of tiles (not pixel)
 * @param y Y coordinate to show the tile at, in number of tiles (not pixel)
 */
gfx_error gfx_tilemap_place(gfx_context* ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y);


/**
 * @brief Wait until the screen in a v-blank state
 *
 * @param context Graphics context, must be initialized
 */
gfx_error gfx_wait_vblank(gfx_context* ctx);


/**
 * @brief Wait until the screen in NOT is v-blank state anymore
 *
 * @param context Graphics context, must be initialized
 */
gfx_error gfx_wait_end_vblank(gfx_context* ctx);


#include "zvb_sprite.h"