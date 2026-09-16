/**
 * SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../gfx_internal.h"

gfx_error gfx_tileset_load_lz(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    if (ctx == NULL || tileset == NULL || size == 0 || ctx->bpp != 8) {
        return GFX_INVALID_ARG;
    }

    uint8_t buffer[TILE_SIZE_8BIT];
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t tile_count = 0;

    while (i < size) {
        const uint8_t byte = tileset[i++];
        const uint8_t type = byte & 0xc0;

        if (type == 0x00) {
            buffer[(uint8_t)j] = byte & 0x3f;
            j++;
        } else if (type == 0x40) {
            uint8_t count = (byte & 0x3f) + 1;
            while (count-- && i < size) {
                buffer[(uint8_t)j] = tileset[i++];
                j++;
            }
        } else if (type == 0x80) {
            const uint8_t length = ((byte & 0x30) >> 4) + 3;
            const uint8_t offset = (byte & 0x0f) + 1;
            for (uint8_t k = 0; k < length; k++) {
                buffer[(uint8_t)j] = buffer[(uint8_t) (j - offset)];
                j++;
            }
        } else {
            const uint8_t length = (byte & 0x3f) + 4;
            const uint8_t offset = tileset[i++] + 1;
            for (uint8_t k = 0; k < length; k++) {
                buffer[(uint8_t)j] = buffer[(uint8_t) (j - offset)];
                j++;
            }
        }

        if (j >= TILE_SIZE_8BIT) {
            gfx_tileset_load_none(ctx, buffer, TILE_SIZE_8BIT,
                                  from + (tile_count * TILE_SIZE_8BIT),
                                  pal_offset, opacity);
            tile_count++;
            j = 0;
        }
    }

    return GFX_SUCCESS;
}
