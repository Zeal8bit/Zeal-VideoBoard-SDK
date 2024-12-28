/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_gfx.h"
#include "zvb_hardware.h"

/**
 * @brief Zeal 8-bit Video Board contains 128 hardware sprites 
 */
#define GFX_SPRITES_COUNT   128

/**
 * @brief In 4bpp mode, the sprite's palette can be choosen via the flag field
 */
#define SPRITE_PALETTE_SHIFT    4
#define SPRITE_PALETTE_MASK     0xf0
#define SPRITE_PALLETE(p)       (((p) & SPRITE_PALETTE_MASK) << SPRITE_PALETTE_SHIFT)

/**
 * @brief Size of the sprite structure, in bytes, without the padding
 */
#define SPRITE_STRUCT_SIZE      6

/**
 * @brief Enumeration of the flags for the sprites. The flags are represented as an
 * 8-bit value, the upper nibble is the palette to use (only valid in 4bpp mode)
 */
typedef enum {
    SPRITE_NONE         = 0,
    SPRITE_TILE_BIT9    = 1 << 0,
    SPRITE_BEHIND_FG    = 1 << 1,
    SPRITE_FLIP_Y       = 1 << 2,
    SPRITE_FLIP_X       = 1 << 3,
    /* The upper nibble is the palette, check the macro above */
} gfx_sprite_flags;


/**
 * @brief Structure of a single sprite. Sprite are 16x16px big and are taken from the same
 * tileset from the (background and foreground) tiles.
 */
typedef struct {
    uint16_t y;     /*!< Y-16 coordinate for the sprite. Write 16 to show it at the top of the screen. */
    uint16_t x;     /*!< X-16 coordinate for the sprite. Write 16 to show it at the left of the screen. */
    uint8_t tile;   /*!< Tile to use for the sprite. In 4bpp, this represents the lowest 8-bit */
    uint8_t flags;  /*!< Flags for the sprite, check `gfx_sprite_flags` enumeration for more details*/
    uint8_t pad[2];
} gfx_sprite;


/**
 * @brief Render a single sprite on screen with the given attributes
 * 
 * @param context Graphics context, must be initialized
 * @param sprite_idx Idnex of the sprite to render
 * @param sprite New attributes for the sprite
 * 
 * @return GFX_INVALID_ARG if the sprite index is not in range or if the context is not initialzied
 *         GFX_SUCCESS on success
 */
gfx_error gfx_sprite_render(gfx_context* ctx, uint8_t sprite_idx, const gfx_sprite* sprite);


/**
 * @brief Render multiple sprites on screen with the given attributes.
 * 
 * @note This function is the same as above but it can manipulate multiple sprites
 */
gfx_error gfx_sprite_render_array(gfx_context* ctx, uint8_t from_idx, const gfx_sprite* sprites, uint8_t length);


/**
 * @brief Set the X coordinate for the given sprite
 */
gfx_error gfx_sprite_set_x(gfx_context* ctx, uint8_t sprite_idx, uint16_t x);


/**
 * @brief Set the Y coordinate for the given sprite
 */
gfx_error gfx_sprite_set_y(gfx_context* ctx, uint8_t sprite_idx, uint16_t y);


/**
 * @brief Set the tile to use for the given sprite. In 4bpp mode, this will only set the
 * lowest 8-bit of the tile index to use.
 */
gfx_error gfx_sprite_set_tile(gfx_context* ctx, uint8_t sprite_idx, uint8_t tile);


/**
 * @brief Set the flags the given sprite, check `gfx_sprite_flags` enumeration
 */
gfx_error gfx_sprite_set_flags(gfx_context* ctx, uint8_t sprite_idx, gfx_sprite_flags flags);