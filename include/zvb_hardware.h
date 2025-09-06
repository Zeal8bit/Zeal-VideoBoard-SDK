/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#define BIT(n)  (1 << (n))

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


/*****************************************************************************/
/*                             I/O Registers                                 */
/*****************************************************************************/

/**
 * @brief The video config module (also named video mapper) is responsible for mapping
 * the I/O component in the I/O bank starting at address 0xA0, up to 0xAF. (16 registers)
 * It also contains the current firmware version of the video board.
 */
#define ZVB_CONFIG_BASE 0x80

IOB(ZVB_CONFIG_BASE + 0x0) zvb_config_ver_rev;
IOB(ZVB_CONFIG_BASE + 0x1) zvb_config_ver_min;
IOB(ZVB_CONFIG_BASE + 0x2) zvb_config_ver_maj;
/* [0x3;0xD] - Reserved */
IOB(ZVB_CONFIG_BASE + 0xe) zvb_config_dev_idx;      // I/O device bank, accessible in 0xA0
IOB(ZVB_CONFIG_BASE + 0xf) zvb_config_phys_addr;    // Physical address start of the video chip


/**
 * @brief Map a peripheral to the banked I/O space (starting at ZVB_PERI_BASE)
 *
 * @param periph Peripheral index, should be any ZVB_PERI_*_IDX macro
 */
static inline void zvb_map_peripheral(uint8_t periph)
{
    zvb_config_dev_idx = periph;
}

/*****************************************************************************/

/**
 * @brief The video control and status module is non-banked, so it is available
 * at anytime for reads and writes. It is responsible for the screen
 * control (mode, enable, scrolling X and Y, etc...) and the screen
 * status. (current raster position, v-blank and h-blank, etc...)
 */
#define ZVB_CTRL_BASE   0x90

/**
 * @brief 16-bit values representing the current raster position (RO).
 * Values latched when LSB read.
 */
IOB(ZVB_CTRL_BASE + 0x0) zvb_ctrl_vpos_low;
IOB(ZVB_CTRL_BASE + 0x1) zvb_ctrl_vpos_high;
IOB(ZVB_CTRL_BASE + 0x2) zvb_ctrl_hpos_low;
IOB(ZVB_CTRL_BASE + 0x3) zvb_ctrl_hpos_high;
/**
 * @brief 16-bit Y scrolling value for Layer0, in GFX mode (R/W).
 * Value latched when MSB written.
 */
IOB(ZVB_CTRL_BASE + 0x4) zvb_ctrl_l0_scr_y_low;
IOB(ZVB_CTRL_BASE + 0x5) zvb_ctrl_l0_scr_y_high;
/**
 * @brief 16-bit X scrolling value for Layer0, in GFX mode (R/W).
 * Value latched when MSB written.
 */
IOB(ZVB_CTRL_BASE + 0x6) zvb_ctrl_l0_scr_x_low;
IOB(ZVB_CTRL_BASE + 0x7) zvb_ctrl_l0_scr_x_high;
/**
 * @brief 16-bit X scrolling value for Layer1, in GFX mode (R/W).
 * Value latched when MSB written.
 */
IOB(ZVB_CTRL_BASE + 0x8) zvb_ctrl_l1_scr_y_low;
IOB(ZVB_CTRL_BASE + 0x9) zvb_ctrl_l1_scr_y_high;
IOB(ZVB_CTRL_BASE + 0xa) zvb_ctrl_l1_scr_x_low;
IOB(ZVB_CTRL_BASE + 0xb) zvb_ctrl_l1_scr_x_high;
/**
 * @brief Video mode register (R/W). Only takes effect after a V-blank occurs.
 */
IOB(ZVB_CTRL_BASE + 0xc) zvb_ctrl_video_mode;

/**
 * @brief Video modes for the register above (Video mode register)
 */
#define ZVB_CTRL_VID_MODE_TEXT_640          0
#define ZVB_CTRL_VID_MODE_TEXT_320          1
#define ZVB_CTRL_VID_MODE_BITMAP_256_MODE   2
#define ZVB_CTRL_VID_MODE_BITMAP_320_MODE   3
#define ZVB_CTRL_VID_MODE_GFX_640_8BIT      4
#define ZVB_CTRL_VID_MODE_GFX_320_8BIT      5
#define ZVB_CTRL_VID_MODE_GFX_640_4BIT      6
#define ZVB_CTRL_VID_MODE_GFX_320_4BIT      7
#define ZVB_CTRL_VID_MODE_GFX_LAST          (ZVB_CTRL_VID_MODE_GFX_320_4BIT)

/**
 * @brief Video mode status. The meaning of each bit is:
 * Bit 0 - Set when in H-blank (RO)
 * Bit 1 - Set when in V-blank (RO)
 * Bit 2 - Enable v-blank interrupts (R/W)
 * Bit 3 - Enable h-blank interrupts (R/W)
 * Bit 4:6 - Reserved
 * Bit 7 - Set to enable screen. Black screen when unset. (R/W)
 */
IOB(ZVB_CTRL_BASE + 0xd) zvb_ctrl_status;

/**
 * @brief Macros for control status register
 */
#define ZVB_CTRL_STATUS_HBLANK_BIT      0
#define ZVB_CTRL_STATUS_VBLANK_BIT      1
#define ZVB_CTRL_STATUS_SCREEN_ON_BIT   7

/**
 * @brief Video Board Interrupt Status and Clear
 * Bit 0 - H-blank interrupt (R/Write-to-clear)
 * Bit 1 - V-blank interrupt (R/Write-to-clear)
 * Bit 2 - GP interrupt (timer or audio) (RO)
 */
IOB(ZVB_CTRL_BASE + 0xe) zvb_ctrl_int_st_clr;

/**
 * @brief Video Board Peripheral Interrupts
 * Bit 0 - Audio interrupt (WO)
 * Bit 1 - Reserved interrupt (WO)
 * Bit 2 - Timer interrupt (WO)
 */
IOB(ZVB_CTRL_BASE + 0xf) zvb_ctrl_int_peri;


/*****************************************************************************/

/**
 * @note All the I/O peripherals defined below can be mapped on the I/O space, at
 * address 0xA0-0xAF. To map them, use the register `zvb_config_dev_idx` define
 * above
 */
#define ZVB_PERI_BASE       0xa0

/* -------- Text controller -------- */

/**
 * @brief Banked Text Control module, usable in text mode (640x480 or 320x240)
 * Used to control the cursor position, color, blinking time, etc...
 */
#define ZVB_PERI_TEXT_IDX   0

/**
 * @note The colors in the registers below represent both the background and foreground colors.
 * The high nibble, bit 7 to bit 4, is the index of the background color.
 * The low nibble, bit 3 to bit 0, is the index of the foreground color.
 * The colors, between 0 and 15, will be taken from the global palette, starting at index 0.
 * Thus, only the first 16 colors are available in text mode.
 */
IOB(ZVB_PERI_BASE + 0x0) zvb_peri_text_print_char;  // (WO) Print a character (taken from the font table) on screen at the current cursor position
IOB(ZVB_PERI_BASE + 0x1) zvb_peri_text_curs_y;      // (R/W) Cursor Y position (in characters count)
IOB(ZVB_PERI_BASE + 0x2) zvb_peri_text_curs_x;      // (R/W) Cursor X position (in characters count)
IOB(ZVB_PERI_BASE + 0x3) zvb_peri_text_scroll_y;    // (R/W) Scroll Y
IOB(ZVB_PERI_BASE + 0x4) zvb_peri_text_scroll_x;    // (R/W) Scroll X
IOB(ZVB_PERI_BASE + 0x5) zvb_peri_text_color;       // (R/W) Current character color (*)
IOB(ZVB_PERI_BASE + 0x6) zvb_peri_text_curs_time;   // (R/W) Blink time, in frames, for the cursor
IOB(ZVB_PERI_BASE + 0x7) zvb_peri_text_curs_char;   // (R/W) Character from the font table for the cursor
IOB(ZVB_PERI_BASE + 0x8) zvb_peri_text_curs_color;  // (R/W) Background and foreground colors for the cursor (*)
IOB(ZVB_PERI_BASE + 0x9) zvb_peri_text_ctrl;        // (R/W) Control register, check the bits below to see what can be achieved

#define ZVB_PERI_TEXT_CTRL_SAVE_CURSOR_BIT      7 // (WO) Save the current cursor position (single save only)
#define ZVB_PERI_TEXT_CTRL_RESTORE_CURSOR_BIT   6 // (WO) Restore the previously saved position
#define ZVB_PERI_TEXT_CTRL_AUTO_SCROLL_X_BIT    5 // (R/W) Enable auto scroll in X when the cursor reaches the end of the line
#define ZVB_PERI_TEXT_CTRL_AUTO_SCROLL_Y_BIT    4 // (R/W) Enable auto scroll in Y when the cursor reaches the end of the screen
#define ZVB_PERI_TEXT_CTRL_WAIT_ON_WRAP_BIT     3 // (R/W) Control whether the cursor wraps directly to the next line once the end
                                                  // of the line is reached or if it ahs to wait for the next character to come first.
                                                  // Useful to implement an eat-newline feature.
#define ZVB_PERI_TEXT_CTRL_SCROLL_Y_OCCUR_BIT   0 // (RO) Set if the previous PRINT_CHAR command (or NEWLINE) triggered a scroll in Y
#define ZVB_PERI_TEXT_CTRL_NEXTLINE             0 // (WO) Make the cursor go to the next line



/* -------- SPI controller -------- */

/**
 * @brief Banked SPI Control module.
 * Used to communicate with the TF card
 */
#define ZVB_PERI_SPI_IDX   1


IOB(ZVB_PERI_BASE + 0x1) zvb_peri_spi_ctrl;        // (R/W) Control register, check the bits defined below

#define ZVB_PERI_SPI_CTRL_START_BIT     7   // (WO) Bit 7 - Start the SPI transaction, can be combined with CS_START bit
#define ZVB_PERI_SPI_CTRL_RESET_BIT     6   // (WO) Bit 6 - Reset the SPI controller (no need to clear this bit)
#define ZVB_PERI_SPI_CTRL_CS_START_BIT  5   // (WO) Bit 5 - When set, CS will go low
#define ZVB_PERI_SPI_CTRL_CS_STOP_BIT   4   // (WO) Bit 4 - When set, CS will go high
#define ZVB_PERI_SPI_CTRL_CS_BIT        3   // (WO) Bit 3 - CS select. 0 for the TF Card, 1 is reserved
                                            // Bit 1 and 2 - Reserved
#define ZVB_PERI_SPI_CTRL_IDLE_BIT      0   // (RO) Bit 0 - Check if the SPI controller is in idle state:
                                            // 1 - In idle state, 0 - transfer is progress

IOB(ZVB_PERI_BASE + 0x2) zvb_peri_spi_clk_div;     // (R/W) SPI clock divider, output frequency is 50/(2*div) MHz

/**
 * @brief The SPI controller presents two 8-byte RAMs, one for incoming data and one for outgoing data. Since these arrays
 * will always be filled with the same amount of bytes, they share the following `len` register.
 * These RAMs can be accessed as regular arrays or via an incremental indexes. This is convenient to send (or receive) data
 * thanks to the `outi`/`otir` instructions (`ini`/`inir` instructions respectively).
 * The following register lets you specify the number of bytes filled in the array.
 * The MSB resets the incremental access indexes, any following write would store the received bytes starting at index 0.
 */
IOB(ZVB_PERI_BASE + 0x3) zvb_peri_spi_ram_len;     // (R/W) Bit 7 - Clears internal FIFO indexes
                                                   //       Bit 0:2 - Number of bytes to send on the next transaction
IOB(ZVB_PERI_BASE + 0x7) zvb_peri_spi_fifo;        // (R/W) FIFO access to the arrays

#define ZVB_PERI_SPI_ARRAY_LEN  8
/**
 * @note SDCC doesn't support defining an array with `__sfr` like the following:
 * IOB(ZVB_PERI_BASE + 0x8) zvb_peri_spi_array[ZVB_PERI_SPI_ARRAY_LEN]; // (RO/WO) Array of data to send / Array of data received
 *                                                                      // On writes, the output array is written, on read, the input array is read
 */
IOB(ZVB_PERI_BASE + 0x8) zvb_peri_spi_array_0;
IOB(ZVB_PERI_BASE + 0x9) zvb_peri_spi_array_1;
IOB(ZVB_PERI_BASE + 0xa) zvb_peri_spi_array_2;
IOB(ZVB_PERI_BASE + 0xb) zvb_peri_spi_array_3;
IOB(ZVB_PERI_BASE + 0xc) zvb_peri_spi_array_4;
IOB(ZVB_PERI_BASE + 0xd) zvb_peri_spi_array_5;
IOB(ZVB_PERI_BASE + 0xe) zvb_peri_spi_array_6;
IOB(ZVB_PERI_BASE + 0xf) zvb_peri_spi_array_7;


/* -------- CRC32 controller -------- */

/**
 * @brief Banked CRC32 Control module.
 * Used to calculate the CRC32 checksum of given flow of data (0x04C11DB7 Poly)
 */
#define ZVB_PERI_CRC_IDX   2


IOB(ZVB_PERI_BASE + 0x0) zvb_peri_crc_ctrl;     // (R/W) Control register, check the bits defined below
#define IO_CRC32_CTRL_RESET_BIT     0   // (WO) Bit 0 - Set this bit to clear the controller (reset the internal state)

IOB(ZVB_PERI_BASE + 0x1) zvb_peri_crc_data_in;  // (WO) Write data to this register, each byte read updates the checksum
IOB(ZVB_PERI_BASE + 0x4) zvb_peri_crc_byte0;    // (RO) Byte 0 of the resulting checksum
IOB(ZVB_PERI_BASE + 0x5) zvb_peri_crc_byte1;    // (RO) Byte 1 of the resulting checksum
IOB(ZVB_PERI_BASE + 0x6) zvb_peri_crc_byte2;    // (RO) Byte 2 of the resulting checksum
IOB(ZVB_PERI_BASE + 0x7) zvb_peri_crc_byte3;    // (RO) Byte 3 of the resulting checksum


/* -------- Sound controller -------- */

/**
 * @brief Banked Sound Control module.
 * There are a total of 4 voices on the Zeal 8-bit Video Board, each of them can generate triangle, square or sawtooth waves,
 * they can also generate noise.
 * There is a fifth voice that can play arbitrary sound thanks to a sample table.
 */
#define ZVB_PERI_SOUND_IDX   3

/* ---- The following registers are only for voices 0 to 3 --- */

/**
 * @brief The resulting frequency, in Hz can be calculated with the formula: (sample_rate * freq) / 2^16
 * Where `sample_rate` is 44091 and `freq` is the 16-bit value defined below (register 0 and 1)
 */
IOB(ZVB_PERI_BASE + 0x0) zvb_peri_sound_freq_low;   // (WO) Low byte of the 16-bit frequency. Value taken into account *after* high byte HIGH byte is written
IOB(ZVB_PERI_BASE + 0x1) zvb_peri_sound_freq_high;  // (WO) High byte of the 16-bit frequency, latches the previously written low byte

/**
 * @brief Lowest 2 bits of this register represent the waveform for the voice, as described by the defines below
 * The highest 3 bits represent the duty-cycle for the square wave waveform (if selected):
 * 0b111 -> 87.5%
 * 0b110 -> 75%
 * 0b101 -> 62.5%
 * 0b100 -> 50%
 * 0b011 -> 37.5%
 * 0b010 -> 25%
 * 0b001 -> 12.5%
 * 0b000 -> 0%
 */
IOB(ZVB_PERI_BASE + 0x2) zvb_peri_sound_wave;   // (WO) Write the lowest 2 bits to choose the waveform
#define ZVB_PERI_SOUND_SQUARE    0
#define ZVB_PERI_SOUND_TRIANGLE  1
#define ZVB_PERI_SOUND_SAWTOOTH  2
#define ZVB_PERI_SOUND_NOISE     3


IOB(ZVB_PERI_BASE + 0x3) zvb_peri_sound_volume;   // (WO) Per Voice Volume, Refer to zvb_peri_sound_master_vol for values

/* ---- The following registers are only for the custom sample table voice (7) --- */

/**
 * @brief The sample table voice has a 256-byte FIFO, writing to this register fills it.
 *        Each sample can be 8-bit or 16-bit (little-endian)
 */
IOB(ZVB_PERI_BASE + 0x0) zvb_peri_sound_sample_fifo;   // (WO)

/**
 * @brief Sample rate divider. This value determines how fast the samples will be read and outputted
 * from the FIFO. The final sample rate will be calculated as: 44091/(divider + 1)
 *
 * For example, to get a sample rate of 44091, this register must be set to 0.
 */
IOB(ZVB_PERI_BASE + 0x1) zvb_peri_sound_sample_div;   // (R/W)

/**
 * @brief Configuration register for this sample voice
 */
IOB(ZVB_PERI_BASE + 0x2) zvb_peri_sound_sample_conf;   // (R/W)
#define ZVB_SAMPLE_CONF_SIZE_BIT    0   // 1: (unsigned) 8 bit mode, 0: 16-bit mode
#define ZVB_SAMPLE_CONF_RSVD_BIT    1
#define ZVB_SAMPLE_CONF_SIGN_BIT    2   // Only for 16-bit mode, ignored for 8-bit mode. 1: signed samples, 0: unsigned samples
#define ZVB_SAMPLE_CONF_INTENA_BIT  3   // (R/W) When 1, interrupts are enabled when FIFO is empty
#define ZVB_SAMPLE_CONF_FULL_BIT    6   // (RO) When 1, the FIFO is full
#define ZVB_SAMPLE_CONF_READY_BIT   7   // (RO) When 1, the FIFO is empty/all samples have been played

/* ---- The following registers are the "master" registers, affecting all registers ---- */


/**
 * @brief (WO) You can assign a voice to a specific channel (left/right) by setting the
 * channel bit on `zvb_peri_sound_volume_left` or `zvb_peri_sound_volume_right` accordingly.
 *
 * Bit 7 is assigned to the sample voice, you can use `sound_voice_t` to combine voices.
 *
 * Example:
 * zvb_peri_sound_volume_left  = VOICE0 | VOICE2; // VOICE0 and VOICE2 are assigned to left channel
 * zvb_peri_sound_volume_right = VOICE1 | VOICE3; // VOICE1 and VOICE3 are assigned to right channel
 *
 */
IOB(ZVB_PERI_BASE + 0xb) zvb_peri_sound_left_channel;
IOB(ZVB_PERI_BASE + 0xc) zvb_peri_sound_right_channel;

/**
 * @brief (WO) Bitmap where bit i holds voice i (1 = on hold, 0 = can output sound)
 * This is convenient to start two or more voices at the exact same time.
 */
IOB(ZVB_PERI_BASE + 0xd) zvb_peri_sound_hold;

/**
 * @brief (WO) Master volume, controls the output of the right and left channels.
 * The register is organized as follows:
 * Bits 0-1: left channel volume, the steps are defined as ZVB_PERI_SOUND_VOL_* macros
 * Bits 2-3: right channel volume, the steps are defined as ZVB_PERI_SOUND_VOL_* macros
 * Bits 4-5: reserved
 * Bit 6: left channel volume disable (0%)
 * Bit 7: right channel volume disable (0%)
 */
IOB(ZVB_PERI_BASE + 0xe) zvb_peri_sound_master_vol;
#define ZVB_PERI_SOUND_VOL_DISABLE  0x80  // Disables master volume (volume = 0)
#define ZVB_PERI_SOUND_VOL_25       0x00  // Volume = 25%
#define ZVB_PERI_SOUND_VOL_50       0x01  // Volume = 50%
#define ZVB_PERI_SOUND_VOL_75       0x02  // Volume = 75%
#define ZVB_PERI_SOUND_VOL_100      0x03  // Volume = 100%

/**
 * @brief Register 0xF is a bitmap that selects/deselects the voice that will be configurable via registers above (frequency and waveform)
 * For example, if reg[0xE] contains 0x0F, all 4 voices will be configured when writes are performed on frequency and/or waveveform registers.
 */
 IOB(ZVB_PERI_BASE + 0xf) zvb_peri_sound_select;    // (R/W)


/* -------- DMA controller -------- */

/**
 * @brief Banked DMA Control module.
 * Used to speed up transfers from/to any physical address.
 */
#define ZVB_PERI_DMA_IDX   4


IOB(ZVB_PERI_BASE + 0x0) zvb_peri_dma_ctrl;     // (R/W) Control register, check the bits defined below
#define ZVB_PERI_DMA_CTRL_START     0x80

IOB(ZVB_PERI_BASE + 0x1) zvb_peri_dma_addr0;  // (WO)
IOB(ZVB_PERI_BASE + 0x2) zvb_peri_dma_addr1;  // (WO)
IOB(ZVB_PERI_BASE + 0x3) zvb_peri_dma_addr2;  // (WO)
IOB(ZVB_PERI_BASE + 0x9) zvb_peri_dma_clk_div;// (WO)

#define ZVB_PERI_DMA_OP_INC         0
#define ZVB_PERI_DMA_OP_DEC         1


/* -------- Timer controller -------- */

/**
 * @brief Banked Timer Control module.
 */
#define ZVB_PERI_TIMER_IDX   6


IOB(ZVB_PERI_BASE + 0x0) zvb_peri_timer_ctrl;     // (R/W) Control register, check the bits defined below
#define ZVB_PERI_TIMER_CTRL_ENABLE  0x80
#define ZVB_PERI_TIMER_CTRL_AUTOREL 0x40
#define ZVB_PERI_TIMER_CTRL_DIR     0x20    // 0 = up, 1 = down
#define ZVB_PERI_TIMER_CTRL_INT_ENA 0x10

#define ZVB_PERI_TIMER_CTRL_RESET   0x01


IOB(ZVB_PERI_BASE + 0x1) zvb_peri_timer_div_lo;  // Clock Divider - LSB (WO)
IOB(ZVB_PERI_BASE + 0x2) zvb_peri_timer_div_hi;  // Clock Divider - MSB (WO)
IOB(ZVB_PERI_BASE + 0x3) zvb_peri_timer_rel_lo;  // Reload value - LSB (WO)
IOB(ZVB_PERI_BASE + 0x4) zvb_peri_timer_rel_hi;  // Reload value - MSB (WO)
IOB(ZVB_PERI_BASE + 0x5) zvb_peri_timer_cnt_lo;  // Counter value - LSB (R/W)
IOB(ZVB_PERI_BASE + 0x6) zvb_peri_timer_cnt_hi;  // Counter value - MSB (R/W)
IOB(ZVB_PERI_BASE + 0x7) zvb_peri_timer_int_st;  // Interrupt status (R/W, write-to-clear)

// Common clock dividers
#define ZVB_PERI_TIMER_CLOCK_DIV_1KHZ    49999
#define ZVB_PERI_TIMER_CLOCK_DIV_10KHZ   4999
#define ZVB_PERI_TIMER_CLOCK_DIV_100MHZ  499
#define ZVB_PERI_TIMER_CLOCK_DIV_1MHZ    49
