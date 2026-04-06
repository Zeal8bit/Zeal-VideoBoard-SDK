/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

static void memaddcpy(uint8_t* dst, uint8_t* src, size_t size, uint8_t opacity, uint8_t offset)
{
    if (offset) {
        while (size) {
            uint8_t byte = *src + offset;
            if (opacity && byte == offset) {
                byte = 0;
            }
            *dst = byte;
            src++;
            dst++;
            size--;
        }
    } else {
        memcpy(dst, src, size);
    }
}

gfx_error gfx_tileset_load(gfx_context* ctx, void* tileset, uint16_t size, const gfx_tileset_options* options)
{
    if (ctx == NULL || tileset == NULL || size == 0) {
        return GFX_INVALID_ARG;
    }

    const uint16_t from = options ? options->from_byte : 0;
    const uint8_t compression = options ? options->compression : 0;
    const uint8_t pal_offset = options ? options->pal_offset : 0;
    const uint8_t opacity = options ? options->opacity : 0;

    uint8_t* vram_tileset = (uint8_t*) VRAM_VIRT_ADDR;
    uint8_t* user_tileset = (uint8_t*) tileset;

    if (compression != TILESET_COMP_NONE) {
        switch (compression) {
        case TILESET_COMP_1BIT:
            return gfx_tileset_load_1bit(ctx, user_tileset, size, from, pal_offset);
        case TILESET_COMP_2BIT:
            return gfx_tileset_load_2bit(ctx, user_tileset, size, from, pal_offset, opacity);
        case TILESET_COMP_4BIT:
            return gfx_tileset_load_4bit(ctx, user_tileset, size, from, pal_offset, opacity);
        case TILESET_COMP_RLE:
            return gfx_tileset_load_rle(ctx, user_tileset, size, from, pal_offset, opacity);
        }
    } else {
        uint8_t page = from / (16 * 1024);
        uint16_t from_byte = from % (16 * 1024);
        uint16_t remaining = size;
        while (remaining) {
            size_t can_copy = 16 * 1024 - from_byte;
            size_t to_copy = MIN(remaining, can_copy);
            gfx_map_tileset(page++);
            memaddcpy(vram_tileset + from_byte, user_tileset, to_copy, opacity, pal_offset);
            user_tileset += to_copy;
            remaining -= to_copy;
            from_byte = 0;
        }

        gfx_demap_vram(ctx->backup_page);
    }
    return GFX_SUCCESS;
}
