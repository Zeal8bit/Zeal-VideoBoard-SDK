/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gfx_internal.h"

gfx_error gfx_palette_load(gfx_context* ctx, void* palette, uint16_t size, uint8_t from)
{
    if (ctx == NULL || palette == NULL || size == 0 || (from * 2 + size) > 512) {
        return GFX_INVALID_ARG;
    }

    uint16_t* vram_palette = (uint16_t*) (VRAM_VIRT_ADDR + VID_MEM_PALETTE_OFFSET);

    gfx_map_vram();
    memcpy(&vram_palette[from], palette, size);
    gfx_demap_vram(ctx->backup_page);
    return GFX_SUCCESS;
}
