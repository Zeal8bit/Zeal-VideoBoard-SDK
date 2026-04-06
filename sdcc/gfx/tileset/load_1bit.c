/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load_1bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset)
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

        if (bpp == 8) {
            /* If the current mode is 256-color, one bit must be converted to one byte */
            for (uint8_t i = 0; i < 8; i++) {
                *vram_tileset++ = pal_offset + ((byte & 0x80) ? 1 : 0);
                byte = byte << 1;
            }
        } else {
            /* Else, we are in 16-color mode, one bit represent one nibble */
            for (uint8_t i = 0; i < 4; i++) {
                const uint8_t left_pix = pal_offset + ((byte & 0x80) ? 1 : 0);
                const uint8_t right_pix = pal_offset + ((byte & 0x40) ? 1 : 0);
                *vram_tileset++ = ((left_pix & 0xf) << 4) | (right_pix & 0xf);
                byte = byte << 2;
            }
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
