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
#define TILESET_COMP_2BIT   3
#define TILESET_COMP_RLE    16


/**
 * @brief Helper to convert an RGB888 color into an RGB565. The result is an unsigned 16-bit value.
 */
#define RGB888_TO_RGB565(r, g, b)  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))


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

/************************
 * INITIALIZE
 ************************/

/**
 * @brief Initialize graphics with the given mode and virtual page to use
 *
 * @param mode Mode choosen from `ZVB_CTRL_VID_MODE_*` defines
 * @param out Initialized graphics context handler
 */
gfx_error gfx_initialize(uint8_t mode, gfx_context* out);

/************************
 * SCREEN
 ************************/

/**
 * @brief Enable or disable the screen
 *
 * @param ena When 0, disables the screen, else enables it
 */
void gfx_enable_screen(uint8_t ena);

/************************
 * PALETTE
 ************************/

/**
 * @brief Load a (part of) color palette in video memory.
 *
 * @note The palette is made out of 256 RGB565 colors.
 *
 * @param ctx Graphics context, must be initialized
 * @param palette Address of the colors to load in video memory
 * @param size Size of the palette to load, in bytes (and not color count)
 * @param from Color index to start loading from. Convenient to replace a part of the palette.
 */
gfx_error gfx_palette_load(gfx_context* ctx, void* palette, uint16_t size, uint8_t from);


/************************
 * TILESET
 ************************/
/**
 * @brief Load a tileset into video memory
 *
 * @note The tileset can be 4-bit or 1-bit compressed. This can be used to reduce the size of the tileset
 *       when the current graphic mode is 8bpp.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure to give more info about the tileset. Can be NULL for raw tileset.
 */
gfx_error gfx_tileset_load(gfx_context* ctx, void* tileset, uint16_t size, const gfx_tileset_options* options);

/**
 * @brief Load raw/uncompressed tileset data.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param from Byte offset in tileset VRAM where data is loaded
 * @param pal_offset Palette offset added to pixels when applicable
 * @param opacity Enable transparency behavior when applicable
 */
gfx_error gfx_tileset_load_none(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity);


/**
 * @brief Load 1-bit-compressed tileset data.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param from Byte offset in tileset VRAM where data is loaded
 * @param pal_offset Palette offset added to pixels
 */
gfx_error gfx_tileset_load_1bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset);

/**
 * @brief Load 2-bit-compressed tileset data.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param from Byte offset in tileset VRAM where data is loaded
 * @param pal_offset Palette offset added to pixels
 * @param opacity Enable transparency behavior for zero-valued pixels
 */
gfx_error gfx_tileset_load_2bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity);

/**
 * @brief Load 4-bit-compressed tileset data.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param from Byte offset in tileset VRAM where data is loaded
 * @param pal_offset Palette offset added to pixels
 * @param opacity Enable transparency behavior for zero-valued pixels
 */
gfx_error gfx_tileset_load_4bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity);

/**
 * @brief Load RLE-compressed tileset data.
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param from Byte offset in tileset VRAM where data is loaded
 * @param pal_offset Palette offset added to decompressed pixels
 * @param opacity Enable transparency behavior for zero-valued pixels
 */
gfx_error gfx_tileset_load_rle(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity);

#if !defined(ZVB_GFX_DISABLE_TILESET_LOAD_MACROS) && defined(TILESET_LOAD_COMPAT) && (TILESET_LOAD_COMPAT == 1)
/**
 * @brief TILESET_LOAD_COMPAT wrapper mode.
 *
 * - User enables by defining TILESET_LOAD_COMPAT to 1 before including this header.
 * - Wrapper macros let specific loaders accept (ctx, tileset, size, options_ptr).
 * - options_ptr must be non-NULL in wrapper mode.
 * - If NULL options behavior is required, call gfx_tileset_load() instead.
 *
 * ZVB_GFX_DISABLE_TILESET_LOAD_MACROS:
 * - Internal SDK escape hatch to prevent wrapper macro substitution.
 * - Useful when global compile flags define TILESET_LOAD_COMPAT for all units.
 */
/**
 * @brief Extract from-byte value from a non-NULL options pointer.
 *
 * @param options Pointer to gfx_tileset_options (must be non-NULL).
 */
#define _GFX_TILESET_OPT_FROM(options)       ((options)->from_byte)
/**
 * @brief Extract palette offset from a non-NULL options pointer.
 *
 * @param options Pointer to gfx_tileset_options (must be non-NULL).
 */
#define _GFX_TILESET_OPT_PAL_OFFSET(options) ((options)->pal_offset)
/**
 * @brief Extract opacity flag from a non-NULL options pointer.
 *
 * @param options Pointer to gfx_tileset_options (must be non-NULL).
 */
#define _GFX_TILESET_OPT_OPACITY(options)    ((options)->opacity)

/**
 * @brief Load raw/uncompressed tileset data (compat wrapper macro).
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure pointer (must be non-NULL in compat mode)
 */
#define gfx_tileset_load_none(ctx, tileset, size, options) \
    gfx_tileset_load_none((ctx), (tileset), (size), _GFX_TILESET_OPT_FROM(options), _GFX_TILESET_OPT_PAL_OFFSET(options), _GFX_TILESET_OPT_OPACITY(options))

/**
 * @brief Load 1-bit-compressed tileset data (compat wrapper macro).
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure pointer (must be non-NULL in compat mode)
 */
#define gfx_tileset_load_1bit(ctx, tileset, size, options) \
    gfx_tileset_load_1bit((ctx), (tileset), (size), _GFX_TILESET_OPT_FROM(options), _GFX_TILESET_OPT_PAL_OFFSET(options))

/**
 * @brief Load 2-bit-compressed tileset data (compat wrapper macro).
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure pointer (must be non-NULL in compat mode)
 */
#define gfx_tileset_load_2bit(ctx, tileset, size, options) \
    gfx_tileset_load_2bit((ctx), (tileset), (size), _GFX_TILESET_OPT_FROM(options), _GFX_TILESET_OPT_PAL_OFFSET(options), _GFX_TILESET_OPT_OPACITY(options))

/**
 * @brief Load 4-bit-compressed tileset data (compat wrapper macro).
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure pointer (must be non-NULL in compat mode)
 */
#define gfx_tileset_load_4bit(ctx, tileset, size, options) \
    gfx_tileset_load_4bit((ctx), (tileset), (size), _GFX_TILESET_OPT_FROM(options), _GFX_TILESET_OPT_PAL_OFFSET(options), _GFX_TILESET_OPT_OPACITY(options))

/**
 * @brief Load RLE-compressed tileset data (compat wrapper macro).
 *
 * @param ctx Graphics context, must be initialized
 * @param tileset Address of the bytes/tileset to load in video memory
 * @param size Size of the tileset array, in bytes
 * @param options Option structure pointer (must be non-NULL in compat mode)
 */
#define gfx_tileset_load_rle(ctx, tileset, size, options) \
    gfx_tileset_load_rle((ctx), (tileset), (size), _GFX_TILESET_OPT_FROM(options), _GFX_TILESET_OPT_PAL_OFFSET(options), _GFX_TILESET_OPT_OPACITY(options))
#endif

/************************
 * TILEMAP
 ************************/

/**
 * @brief Create a tile with a given color
 *
 * @param ctx Graphics context, must be initialized
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
 * @param ctx Graphics context, must be initialized
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
 * @param ctx Graphics context, must be initialized
 * @param tile Index of the tile to show on screen
 * @param layer Layer to show the tile on, must be 0 or 1
 * @param x X coordinate to show the tile at, in number of tiles (not pixel)
 * @param y Y coordinate to show the tile at, in number of tiles (not pixel)
 */
gfx_error gfx_tilemap_place(gfx_context* ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y);

/************************
 * VBLANK/VSYNC
 ************************/

/**
 * @brief Wait until the screen in a v-blank state
 *
 * @param ctx Graphics context, must be initialized
 */
gfx_error gfx_wait_vblank(gfx_context* ctx);


/**
 * @brief Wait until the screen in NOT is v-blank state anymore
 *
 * @param ctx Graphics context, must be initialized
 */
gfx_error gfx_wait_end_vblank(gfx_context* ctx);


#include "zvb_sprite.h"
