/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "zvb_gfx.h"

/**
 * @brief VRAM will be mapped in page 0, which starts at address 0...
 */
#define VRAM_VIRT_ADDR  (0x0000)

#define MAX_LINE        39
#define MAX_COL         79
#define TILE_SIZE_8BIT  256
// #define TILE_SIZE_4BIT  128
// #define TILE_SIZE_1BIT  32

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

/* Workaround to get the page 0 value from the MMU */
const __sfr __banked __at(0xF0) mmu_page0_ro;
__sfr __at(0xF0) mmu_page0;
__sfr __banked __at(0x9d) vid_ctrl_status;

/**
 * @brief Map the VRAM to the first page (page0)
 */
static inline void gfx_map_vram(void)
{
    __asm__ ("di");
    mmu_page0 = VID_MEM_PHYS_ADDR_START >> 14;
}

/**
 * @brief Similarly but for the tileset (16KB at once, max 4 parts)
 */
static inline void gfx_map_tileset(uint8_t part)
{
    __asm__ ("di");
    mmu_page0 = (VID_MEM_TILESET_ADDR >> 14) + part;
}


static inline void gfx_demap_vram(const uint8_t os)
{
    mmu_page0 = os;
    __asm__ ("ei");
}

static void memset_vram(void* ptr, int a, uint16_t size) __naked
{
    (void) ptr;
    (void) a;
    (void) size;
__asm
    ld a, e
    pop de
    pop bc
    push de
    ld e, a
    ; BC has the size now
_memset_vram_loop:
    ld a, b
    or c
    ret z
    ld (hl), e
    inc hl
    dec bc
    jp _memset_vram_loop
__endasm;
}


gfx_error gfx_initialize(uint8_t mode, gfx_context* out)
{
    if (out == NULL ||
        mode > ZVB_CTRL_VID_MODE_GFX_LAST || mode < ZVB_CTRL_VID_MODE_GFX_640_8BIT)
    {
        return GFX_INVALID_ARG;
    }
    out->video_mode = mode;
    /* 8bpp if mode 4 and 5, 4bpp else (6 and 7) */
    out->bpp = (mode & 2) ? 4 : 8;
    out->alpha_idx = 0;
    /* Backup of the virtual page 0 (where the OS is mapped) */
    out->backup_page = mmu_page0_ro;

    uint8_t* vram = VRAM_VIRT_ADDR;
    /* Create an empty tile */
    gfx_map_tileset(0);
    memset_vram(vram, 0, 256); // 256 would create 2 tiles for 4-bit mode
    /* Set the palette color 0 to alpha/black */
    gfx_map_vram();
    memset_vram(vram + VID_MEM_LAYER0_OFFSET, 0, MAX_LINE * MAX_COL);
    // memset_vram(vram + VID_MEM_LAYER1_OFFSET, 0, MAX_LINE * MAX_COL);
    /* Set the palette first color */
    vram[VID_MEM_PALETTE_OFFSET] = 0x00;
    vram[VID_MEM_PALETTE_OFFSET + 1] = 0x00;

    /* Switch mode */
    zvb_ctrl_video_mode = mode;

    gfx_demap_vram(out->backup_page);

    return GFX_SUCCESS;
}


void gfx_enable_screen(uint8_t ena)
{
    vid_ctrl_status = ena ? 1 << 7 : 0;
}


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


static gfx_error gfx_tileset_load_bitmap(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset)
{
    uint8_t  page = from / (16*1024);
    uint16_t from_byte = from % (16*1024);
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
                const uint8_t left_pix  = pal_offset + ((byte & 0x80) ? 1 : 0);
                const uint8_t right_pix = pal_offset + ((byte & 0x40) ? 1 : 0);
                *vram_tileset++ = ((left_pix  & 0xf) << 4) |
                                   (right_pix & 0xf);
                byte = byte << 2;
            }
        }
        /* Each we reached the end of the page, start all over again */
        if ((uintptr_t) vram_tileset & (16*1024) != 0) {
            gfx_map_tileset(++page);
            vram_tileset = VRAM_VIRT_ADDR;
        }
    }

    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}


static gfx_error gfx_tileset_load_2bit(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    uint8_t  page = from / (16*1024);
    uint16_t from_byte = from % (16*1024);
    uint16_t remaining = size;
    gfx_map_tileset(page);
    uint8_t* vram_tileset = (uint8_t*) (VRAM_VIRT_ADDR + from_byte);
    const uint8_t bpp = ctx->bpp;
    (void) opacity;

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
        if ((uintptr_t) vram_tileset & (16*1024) != 0) {
            gfx_map_tileset(++page);
            vram_tileset = VRAM_VIRT_ADDR;
        }
    }

    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}


static gfx_error gfx_tileset_load_nibble(gfx_context* ctx, uint8_t* tileset, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity)
{
    uint8_t  page = from / (16*1024);
    uint16_t from_byte = from % (16*1024);
    uint16_t remaining = size;
    gfx_map_tileset(page);
    uint8_t* vram_tileset = (uint8_t*) (VRAM_VIRT_ADDR + from_byte);

    while (remaining) {
        const uint8_t byte = *tileset;
        uint8_t high = (byte >> 4) + pal_offset;
        uint8_t low  = (byte & 0xf) + pal_offset;
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
        if ((uintptr_t) vram_tileset & (16*1024) != 0) {
            gfx_map_tileset(++page);
            vram_tileset = VRAM_VIRT_ADDR;
        }
    }

    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}

static gfx_error gfx_tileset_load_rle(gfx_context* ctx, uint8_t* data, uint16_t size, uint16_t from, uint8_t pal_offset, uint8_t opacity) {
    uint8_t buffer[TILE_SIZE_8BIT + 16];
    uint8_t length = 0;
    uint16_t i = 0; // data index
    uint16_t j = 0; // buffer index
    uint16_t length_of_data = size - 1;

    uint16_t tile_count = 0;

    while (i < length_of_data) {
        length = data[i]; // RLE byte
        i++; // every other byte
        if(length >= 0x80) {
            length = (length - 0x80) + 1;
            uint8_t value = data[i];
            while(length--) {
                buffer[j++] = value;
            }
            i++;
        } else {
            length++;
            memcpy(&buffer[j], &data[i], length);
            i += length;
            j += length;
        }

        if(j >= TILE_SIZE_8BIT) {
            gfx_tileset_options options = {
                .compression = TILESET_COMP_NONE, // load uncompressed data
                .from_byte = from + (tile_count * TILE_SIZE_8BIT), // offset by the current tile index?
                .pal_offset = pal_offset, // copy over
                .opacity = opacity, // copy over
            };
            gfx_tileset_load(ctx, &buffer, TILE_SIZE_8BIT,  &options);
            tile_count++;
            j = 0;
        }
    }

    return GFX_SUCCESS;
}

void memaddcpy(uint8_t* dst, uint8_t* src, size_t size, uint8_t opacity, uint8_t offset)
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
    /* Count the number of mappings required */
    const uint16_t from = options ? options->from_byte : 0;
    const uint8_t compression = options ? options->compression : 0;
    const uint8_t pal_offset = options ? options->pal_offset : 0;
    const uint8_t opacity = options ? options->opacity : 0;

    uint8_t* vram_tileset = (uint8_t*) VRAM_VIRT_ADDR;
    uint8_t* user_tileset = (uint8_t*) tileset;

    if (compression != TILESET_COMP_NONE) {
        switch(compression) {
        case TILESET_COMP_1BIT:
            return gfx_tileset_load_bitmap(ctx, user_tileset, size, from, pal_offset);
        case TILESET_COMP_2BIT:
            return gfx_tileset_load_2bit(ctx, user_tileset, size, from, pal_offset, opacity);
        case TILESET_COMP_4BIT:
            if (ctx->bpp == 8)
                return gfx_tileset_load_nibble(ctx, user_tileset, size, from, pal_offset, opacity);
            return GFX_INVALID_ARG;
        case TILESET_COMP_RLE:
            if (ctx->bpp == 8)
                return gfx_tileset_load_rle(ctx, user_tileset, size, from, pal_offset, opacity);
            return GFX_INVALID_ARG;
        }
    } else {
        /* Calculate the number of 16KB pages we will write to VRAM */
        uint8_t  page = from / (16*1024);
        uint16_t from_byte = from % (16*1024);
        uint16_t remaining = size;
        while (remaining) {
            /* Maximum number of bytes that can be copied in the current page */
            size_t can_copy = 16*1024 - from_byte;
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


gfx_error gfx_tilemap_load(gfx_context* ctx, void* tiles, uint8_t size, uint8_t layer, uint8_t x, uint8_t y)
{
    if (ctx == NULL || tiles == NULL || size == 0) {
        return GFX_INVALID_ARG;
    }

    uint16_t layer_offset = layer != 0 ? VID_MEM_LAYER1_OFFSET : 0;
    uint16_t position = y * (MAX_COL + 1) + x;
    uint8_t* vram_tilemap = (uint8_t*) (VRAM_VIRT_ADDR + layer_offset + position);
    gfx_map_vram();
    memcpy(vram_tilemap, tiles, size);
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}



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


gfx_error gfx_wait_vblank(gfx_context* ctx)
{
    (void) ctx;
    while((vid_ctrl_status & 2) == 0) {
    }
    return GFX_SUCCESS;
}

gfx_error gfx_wait_end_vblank(gfx_context* ctx)
{
    (void) ctx;
    while(vid_ctrl_status & 2) {
    }
    return GFX_SUCCESS;
}


/**
 * Sprites related functions
 */

gfx_error gfx_sprite_render(gfx_context* ctx, uint8_t sprite_idx, const gfx_sprite* sprite)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL || sprite == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &wo_sprites[sprite_idx];

    gfx_map_vram();
    memcpy(destination, sprite, SPRITE_STRUCT_SIZE);
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}


gfx_error gfx_sprite_render_array(gfx_context* ctx, uint8_t from_idx, const gfx_sprite* sprites, uint8_t length)
{
    /* Making these two pointers static will make the code faster since they won't be on the stack */
    static gfx_sprite* destination;
    static const gfx_sprite* src;

    if (from_idx + length > GFX_SPRITES_COUNT || ctx == NULL || sprites == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;

    destination = &wo_sprites[from_idx];
    src = sprites;

    uint8_t backup_page = ctx->backup_page;
    for (uint8_t i = 0; i < length; i++) {
        /**
         * We cannot use a single memcpy for two reasons:
         * - Two bytes of the structure are padding, so it would be unnecessary bytes copied
         * - We must not hold the interrupt for too long
         */
        gfx_map_vram();
        memcpy(destination, src, SPRITE_STRUCT_SIZE);
        gfx_demap_vram(backup_page);
        destination++;
        src++;
    }

    return GFX_SUCCESS;
}

gfx_error gfx_sprite_set_x(gfx_context* ctx, uint8_t sprite_idx, uint16_t x)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &wo_sprites[sprite_idx];
    gfx_map_vram();
    destination->x = x;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}

gfx_error gfx_sprite_set_y(gfx_context* ctx, uint8_t sprite_idx, uint16_t y)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &wo_sprites[sprite_idx];
    gfx_map_vram();
    destination->y = y;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}

gfx_error gfx_sprite_set_tile(gfx_context* ctx, uint8_t sprite_idx, uint8_t tile)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &wo_sprites[sprite_idx];
    gfx_map_vram();
    destination->tile = tile;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}

gfx_error gfx_sprite_set_flags(gfx_context* ctx, uint8_t sprite_idx, gfx_sprite_flags flags)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is write-only */
    gfx_sprite* wo_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &wo_sprites[sprite_idx];
    gfx_map_vram();
    destination->flags = flags;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}