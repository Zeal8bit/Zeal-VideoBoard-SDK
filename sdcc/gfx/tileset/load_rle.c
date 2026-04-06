/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load_rle(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    if (ctx == NULL || tileset == NULL || size == 0 || ctx->bpp != 8) {
        return GFX_INVALID_ARG;
    }

    uint8_t buffer[TILE_SIZE_8BIT + 16];
    uint8_t length = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t length_of_data = size - 1;

    uint16_t tile_count = 0;

    while (i < length_of_data) {
        length = tileset[i];
        i++;
        if (length >= 0x80) {
            length = (length - 0x80) + 1;
            uint8_t value = tileset[i];
            while (length--) {
                buffer[j++] = value;
            }
            i++;
        } else {
            length++;
            memcpy(&buffer[j], &tileset[i], length);
            i += length;
            j += length;
        }

        if (j >= TILE_SIZE_8BIT) {
            gfx_tileset_options options = {
                .compression = TILESET_COMP_NONE,
                .from_byte = from + (tile_count * TILE_SIZE_8BIT),
                .pal_offset = pal_offset,
                .opacity = opacity,
            };
            gfx_tileset_load(ctx, &buffer, TILE_SIZE_8BIT, &options);
            tile_count++;
            j = 0;
        }
    }

    return GFX_SUCCESS;
}
