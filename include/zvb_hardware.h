/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#define IOB(addr) __sfr __at(addr)

/**
 * @brief Physical addresses for the video memory
 */
#define VID_MEM_PHYS_ADDR_START     0x100000

#define VID_MEM_LAYER0_OFFSET       0
#define VID_MEM_PALETTE_OFFSET      0xE00
#define VID_MEM_LAYER1_OFFSET       0x1000
#define VID_MEM_SPRITE_OFFSET       0x2800
#define VID_MEM_FONT_OFFSET         0x3000
#define VID_MEM_TILESET_OFFSET      0x10000

#define VID_MEM_LAYER0_ADDR         (VID_MEM_PHYS_ADDR_START + VID_MEM_LAYER0_OFFSET)
#define VID_MEM_PALETTE_ADDR        (VID_MEM_PHYS_ADDR_START + VID_MEM_PALETTE_OFFSET)
#define VID_MEM_LAYER1_ADDR         (VID_MEM_PHYS_ADDR_START + VID_MEM_LAYER1_OFFSET)
#define VID_MEM_SPRITE_ADDR         (VID_MEM_PHYS_ADDR_START + VID_MEM_SPRITE_OFFSET)
#define VID_MEM_FONT_ADDR           (VID_MEM_PHYS_ADDR_START + VID_MEM_FONT_OFFSET)
#define VID_MEM_TILESET_ADDR        (VID_MEM_PHYS_ADDR_START + VID_MEM_TILESET_OFFSET)


#define ZVB_CONFIG_BASE 0x80

IOB(ZVB_CONFIG_BASE + 0x0) zvb_config_ver_rev;
IOB(ZVB_CONFIG_BASE + 0x1) zvb_config_ver_min;
IOB(ZVB_CONFIG_BASE + 0x2) zvb_config_ver_maj;
IOB(ZVB_CONFIG_BASE + 0xe) zvb_config_dev_idx;
IOB(ZVB_CONFIG_BASE + 0xf) zvb_config_phys_addr;


#define ZVB_CTRL_BASE   0x90

IOB(ZVB_CTRL_BASE + 0x0) zvb_ctrl_vpos_low;
IOB(ZVB_CTRL_BASE + 0x1) zvb_ctrl_vpos_high;
IOB(ZVB_CTRL_BASE + 0x2) zvb_ctrl_hpos_low;
IOB(ZVB_CTRL_BASE + 0x3) zvb_ctrl_hpos_high;
IOB(ZVB_CTRL_BASE + 0x4) zvb_ctrl_l0_scr_y_low;
IOB(ZVB_CTRL_BASE + 0x5) zvb_ctrl_l0_scr_y_high;
IOB(ZVB_CTRL_BASE + 0x6) zvb_ctrl_l0_scr_x_low;
IOB(ZVB_CTRL_BASE + 0x7) zvb_ctrl_l0_scr_x_high;
IOB(ZVB_CTRL_BASE + 0x8) zvb_ctrl_l1_scr_y_low;
IOB(ZVB_CTRL_BASE + 0x9) zvb_ctrl_l1_scr_y_high;
IOB(ZVB_CTRL_BASE + 0xa) zvb_ctrl_l1_scr_x_low;
IOB(ZVB_CTRL_BASE + 0xb) zvb_ctrl_l1_scr_x_high;

IOB(ZVB_CTRL_BASE + 0xc) zvb_ctrl_video_mode;

#define ZVB_CTRL_VID_MODE_TEXT_640      0
#define ZVB_CTRL_VID_MODE_TEXT_320      1
#define ZVB_CTRL_VID_MODE_GFX_640_8BIT  4
#define ZVB_CTRL_VID_MODE_GFX_320_8BIT  5
#define ZVB_CTRL_VID_MODE_GFX_640_4BIT  6
#define ZVB_CTRL_VID_MODE_GFX_320_4BIT  7
#define ZVB_CTRL_VID_MODE_GFX_LAST      7

IOB(ZVB_CTRL_BASE + 0xd) zvb_ctrl_status;

#define ZVB_CTRL_STATUS_HBLANK_BIT  0
#define ZVB_CTRL_STATUS_VBLANK_BIT  1
#define ZVB_CTRL_STATUS_SCREEN_ON   7
