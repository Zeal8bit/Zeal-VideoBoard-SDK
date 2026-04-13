/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Legacy compatibility translation unit.
 *
 * The implementation has been split into the sdcc/sound module files for
 * CMake
 * builds. Keep this file as a shim so existing Makefile-based workflows that
 * compile sdcc/zvb_sound.c directly continue to work.
 */

#include "sound/core.c"
#include "sound/voices.c"
#include "sound/channels.c"
#include "sound/volume.c"
#include "sound/samples.c"
