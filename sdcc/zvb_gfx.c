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

#define MAX_LINE    39
#define MAX_COL     79

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


static gfx_error gfx_tileset_load_bitmap(gfx_context* ctx, void* tileset, uint16_t size, uint8_t from, uint8_t pal_offset)
{
    // TODO
    (void) ctx;
    (void) tileset;
    (void) size;
    (void) from;
    (void) pal_offset;
    return GFX_FAILURE;
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

void memaddcpy(uint8_t* dst, uint8_t* src, size_t size, uint8_t offset)
{
    if (offset) {
        while (size) {
            *dst = *src + offset;
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

    if (ctx->bpp == 8 && compression != TILESET_COMP_NONE) {
        if (compression == TILESET_COMP_1BIT)
            return gfx_tileset_load_bitmap(ctx, tileset, size, from, pal_offset);
        if (options->compression == TILESET_COMP_4BIT)
            return gfx_tileset_load_nibble(ctx, tileset, size, from, pal_offset, opacity);

    } else {
        /* Calculate the number of 16KB pages we will write to VRAM */
        uint8_t  page = from / 16*1024;
        uint16_t from_byte = from % 16*1024;
        uint16_t remaining = size;
        while (remaining) {
            /* Maximum number of bytes that can be copied in the current page */
            size_t can_copy = 16*1024 - from_byte;
            size_t to_copy = MIN(remaining, can_copy);
            gfx_map_tileset(page++);
            memaddcpy(vram_tileset + from_byte, tileset, to_copy, pal_offset);
            tileset += to_copy;
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
    while((vid_ctrl_status & 2) == 1) {
    }
    return GFX_SUCCESS;
}
