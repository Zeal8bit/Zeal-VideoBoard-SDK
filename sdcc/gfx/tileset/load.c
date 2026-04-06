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

static gfx_error tileset_load_1bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset)
{
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

static gfx_error tileset_load_2bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
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

static gfx_error tileset_load_4bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
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

static gfx_error tileset_load_rle(gfx_context* ctx, uint8_t* data, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    uint8_t buffer[TILE_SIZE_8BIT + 16];
    uint8_t length = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t length_of_data = size - 1;

    uint16_t tile_count = 0;

    while (i < length_of_data) {
        length = data[i];
        i++;
        if (length >= 0x80) {
            length = (length - 0x80) + 1;
            uint8_t value = data[i];
            while (length--) {
                buffer[j++] = value;
            }
            i++;
        } else {
            length++;
            memcpy(&buffer[j], &data[i], length);
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
            return tileset_load_1bit(ctx, user_tileset, size, from, pal_offset);
        case TILESET_COMP_2BIT:
            return tileset_load_2bit(ctx, user_tileset, size, from, pal_offset, opacity);
        case TILESET_COMP_4BIT:
            if (ctx->bpp == 8) {
                return tileset_load_4bit(ctx, user_tileset, size, from, pal_offset, opacity);
            }
            return GFX_INVALID_ARG;
        case TILESET_COMP_RLE:
            if (ctx->bpp == 8) {
                return tileset_load_rle(ctx, user_tileset, size, from, pal_offset, opacity);
            }
            return GFX_INVALID_ARG;
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
