/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tilemap_place(gfx_context* ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y)
{
    if (ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    const uint16_t layer_offset = (layer == 0) ? 0 : VID_MEM_LAYER1_OFFSET;
    const uint16_t position = y * (MAX_COL + 1) + x;
    uint8_t* vram_tilemap = (uint8_t*) (VRAM_VIRT_ADDR + layer_offset + position);
    gfx_map_vram();
    *vram_tilemap = tile;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}
