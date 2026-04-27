/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load(gfx_context* ctx, void* tileset, uint16_t size, const gfx_tileset_options* options)
{
    if (ctx == NULL || tileset == NULL || size == 0) {
        return GFX_INVALID_ARG;
    }

    const uint8_t compression = options ? options->compression : 0;
    const uint16_t from = options ? options->from_byte : 0;
    const uint8_t pal_offset = options ? options->pal_offset : 0;
    const uint8_t opacity = options ? options->opacity : 0;

    switch (compression) {
    case TILESET_COMP_NONE:
        return gfx_tileset_load_none(ctx, (uint8_t*) tileset, size, from, pal_offset, opacity);
    case TILESET_COMP_1BIT:
        return gfx_tileset_load_1bit(ctx, (uint8_t*) tileset, size, from, pal_offset);
    case TILESET_COMP_2BIT:
        return gfx_tileset_load_2bit(ctx, (uint8_t*) tileset, size, from, pal_offset, opacity);
    case TILESET_COMP_4BIT:
        return gfx_tileset_load_4bit(ctx, (uint8_t*) tileset, size, from, pal_offset, opacity);
    case TILESET_COMP_RLE:
        return gfx_tileset_load_rle(ctx, (uint8_t*) tileset, size, from, pal_offset, opacity);
    }
    return GFX_SUCCESS;
}
