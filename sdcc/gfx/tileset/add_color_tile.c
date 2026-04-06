/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_add_color_tile(gfx_context* ctx, uint16_t index, uint8_t color)
{
    if (ctx == NULL) {
        return GFX_INVALID_ARG;
    }
    uint8_t* vram_tileset = (uint8_t*) VRAM_VIRT_ADDR;
    uint8_t page;
    uint16_t size;
    if (ctx->bpp == 8) {
        size = 256;
        /* Divide by 64 since we have 64 tiles per 16KB pages */
        page = index >> 6;
        /* Shift 8 times since each tile is 256 bytes big */
        vram_tileset += (index & 63) << 8;
    } else if (ctx->bpp == 4) {
        size = 128;
        page = index >> 7;
        vram_tileset += (index & 127) << 7;
    } else {
        return GFX_FAILURE;
    }
    gfx_map_tileset(page);
    memset_vram(vram_tileset, color, size);
    gfx_demap_vram(ctx->backup_page);
    return GFX_SUCCESS;
}
