/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Legacy compatibility translation unit.
 *
 * The implementation has been split into sdcc/gfx/*.c modules for CMake builds.
 * Keep this file as a shim so existing Makefile-based workflows that compile
 * sdcc/zvb_gfx.c directly continue to work.
 */

#include "gfx/core.c"
#include "gfx/palette.c"
#include "gfx/tilemap/load.c"
#include "gfx/tilemap/place.c"
#include "gfx/tileset/load.c"
#include "gfx/tileset/add_color_tile.c"
#include "gfx/sprite/render.c"
#include "gfx/sprite/render_array.c"
#include "gfx/sprite/set_xy.c"
#include "gfx/sprite/set_x.c"
#include "gfx/sprite/set_y.c"
#include "gfx/sprite/set_tile.c"
#include "gfx/sprite/set_flags.c"
