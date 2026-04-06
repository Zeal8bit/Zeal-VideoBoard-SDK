/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load_2bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    if (ctx == NULL || tileset == NULL || size == 0) {
        return GFX_INVALID_ARG;
    }

    uint8_t page = from / (16 * 1024);
    uint16_t from_byte = from % (16 * 1024);
    gfx_map_tileset(page);
    uint8_t* vram_tileset = (uint8_t*) (VRAM_VIRT_ADDR + from_byte);
    const uint8_t bpp = ctx->bpp;

    while (size--) {
        uint8_t byte = *tileset++;

        const uint8_t pix_3 = pal_offset + ((byte >> 0) & 3);
        const uint8_t pix_2 = pal_offset + ((byte >> 2) & 3);
        const uint8_t pix_1 = pal_offset + ((byte >> 4) & 3);
        const uint8_t pix_0 = pal_offset + ((byte >> 6) & 3);

        if (bpp == 8) {
            *vram_tileset++ = (opacity && pix_0 == pal_offset) ? 0 : pix_0;
            *vram_tileset++ = (opacity && pix_1 == pal_offset) ? 0 : pix_1;
            *vram_tileset++ = (opacity && pix_2 == pal_offset) ? 0 : pix_2;
            *vram_tileset++ = (opacity && pix_3 == pal_offset) ? 0 : pix_3;
        } else {
            *vram_tileset++ = ((pix_0 & 0xf) << 4) | (pix_1 & 0xf);
            *vram_tileset++ = ((pix_2 & 0xf) << 4) | (pix_3 & 0xf);
        }

        /* Each we reached the end of the page, start all over again */
        if ((uintptr_t) vram_tileset & (16 * 1024) != 0) {
            gfx_map_tileset(++page);
            vram_tileset = VRAM_VIRT_ADDR;
        }
    }

    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}
