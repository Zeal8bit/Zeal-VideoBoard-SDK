/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load_4bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    if (ctx == NULL || tileset == NULL || size == 0 || ctx->bpp != 8) {
        return GFX_INVALID_ARG;
    }

    uint8_t page = from / (16 * 1024);
    uint16_t from_byte = from % (16 * 1024);
    uint16_t remaining = size;
    gfx_map_tileset(page);
    uint8_t* vram_tileset = (uint8_t*) (VRAM_VIRT_ADDR + from_byte);

    while (remaining) {
        const uint8_t byte = *tileset;
        uint8_t high = (byte >> 4) + pal_offset;
        uint8_t low = (byte & 0xf) + pal_offset;
        if (opacity && high == pal_offset) {
            high = 0;
        }
        if (opacity && low == pal_offset) {
            low = 0;
        }
        *vram_tileset = high;
        vram_tileset++;
        *vram_tileset = low;
        vram_tileset++;
        tileset++;
        remaining--;

        /* Each we reached the end of the page, start all over again */
        if ((uintptr_t) vram_tileset & (16 * 1024) != 0) {
            gfx_map_tileset(++page);
            vram_tileset = VRAM_VIRT_ADDR;
        }
    }

    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}
