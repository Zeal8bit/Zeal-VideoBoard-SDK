/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zvb_sprite.h"
#include "../gfx_internal.h"

gfx_error gfx_sprite_set_tile(gfx_context* ctx, uint8_t sprite_idx, uint8_t tile)
{
    if (sprite_idx >= GFX_SPRITES_COUNT || ctx == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is read-write */
    gfx_sprite* rw_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;
    gfx_sprite* destination = &rw_sprites[sprite_idx];
    gfx_map_vram();
    destination->tile = tile;
    destination->flags = destination->flags;
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}
