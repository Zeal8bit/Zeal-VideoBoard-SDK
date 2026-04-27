/**
 * SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ZVB_GFX_DISABLE_TILESET_LOAD_MACROS
#include "zvb_gfx.h"
#undef ZVB_GFX_DISABLE_TILESET_LOAD_MACROS

/**
 * @brief VRAM will be mapped in page 0, which starts at address 0.
 */
#define VRAM_VIRT_ADDR  (0x0000)

#define MAX_LINE        39
#define MAX_COL         79
#define TILE_SIZE_8BIT  256

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

/* Workaround to get the page 0 value from the MMU.
 * Keep these as TU-local SFR declarations so split-object builds do not
 * require external _mmu_page0/_vid_ctrl_status symbols at link time.
 */
static const __sfr __banked __at(0x00f0) mmu_page0_ro;
static __sfr __at(0xf0) mmu_page0;
static __sfr __at(0x9d) vid_ctrl_status;

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

void memset_vram(void* ptr, int a, uint16_t size);
