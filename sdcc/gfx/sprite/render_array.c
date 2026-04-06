/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zvb_sprite.h"
#include "../gfx_internal.h"

gfx_error gfx_sprite_render_array(gfx_context* ctx, uint8_t from_idx, const gfx_sprite* sprites, uint8_t length)
{
    if (from_idx + length > GFX_SPRITES_COUNT || ctx == NULL || sprites == NULL) {
        return GFX_INVALID_ARG;
    }

    /* The sprite VRAM is read-write */
    gfx_sprite* rw_sprites = (gfx_sprite*) VID_MEM_SPRITE_ADDR;

    gfx_map_vram();
    memcpy(&rw_sprites[from_idx], sprites, length * sizeof(gfx_sprite));
    gfx_demap_vram(ctx->backup_page);

    return GFX_SUCCESS;
}
