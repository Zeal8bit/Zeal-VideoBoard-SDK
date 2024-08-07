; SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
;
; SPDX-License-Identifier: Apache-2.0

; This file is meant to be included by assembly files (then assembled with z88dk-z80asm)

    IFNDEF VIDEO_H
    DEFINE VIDEO_H

    ; -------------------------------------------------------------------------- ;
    ;                          Hardware mapping related                          ;
    ; -------------------------------------------------------------------------- ;

    ; Physical address for the memory components.
    ; It is also possible to access the I/O components via the memory bus, but for the
    ; sake of simplicity, we don't do it here.
    DEFC VID_MEM_PHYS_ADDR_START  = 0x100000

    DEFC VID_MEM_LAYER0_OFFSET    = 0x0000
    DEFC VID_MEM_PALETTE_OFFSET   = 0x0E00
    DEFC VID_MEM_LAYER1_OFFSET    = 0x1000
    DEFC VID_MEM_SPRITES_OFFSET   = 0x2800
    DEFC VID_MEM_FONT_OFFSET      = 0x3000
    DEFC VID_MEM_TILESET_OFFSET   = 0x10000

    DEFC VID_MEM_LAYER0_ADDR      = VID_MEM_PHYS_ADDR_START + VID_MEM_LAYER0_OFFSET
    DEFC VID_MEM_PALETTE_ADDR     = VID_MEM_PHYS_ADDR_START + VID_MEM_PALETTE_OFFSET
    DEFC VID_MEM_LAYER1_ADDR      = VID_MEM_PHYS_ADDR_START + VID_MEM_LAYER1_OFFSET
    DEFC VID_MEM_SPRITES_ADDR     = VID_MEM_PHYS_ADDR_START + VID_MEM_SPRITES_OFFSET
    DEFC VID_MEM_FONT_ADDR        = VID_MEM_PHYS_ADDR_START + VID_MEM_FONT_OFFSET
    DEFC VID_MEM_TILESET_ADDR     = VID_MEM_PHYS_ADDR_START + VID_MEM_TILESET_OFFSET


    ; Physical address for the I/O components.
    ; The video mapper is responsible for mapping the I/O component in the I/O bank
    ; starting at address 0xA0, up to 0xAF (16 registers)
    ; It also contains the current firmware version of the video board.
    DEFC VID_IO_MAPPER       = 0x80
        DEFC IO_MAPPER_REV   = VID_IO_MAPPER + 0x0
        DEFC IO_MAPPER_MIN   = VID_IO_MAPPER + 0x1
        DEFC IO_MAPPER_MAJ   = VID_IO_MAPPER + 0x2
        ; [0x3;0xD] - Reserved
        DEFC IO_MAPPER_BANK  = VID_IO_MAPPER + 0xE ; I/O device bank, accessible in 0xA0
        DEFC IO_MAPPER_PHYS  = VID_IO_MAPPER + 0xF ; Physical address start of the video chip


    ; The video control and status module is non-banked, so it is available at anytime for reads
    ; and writes. It is reponsible for the screen control (mode, enable, scrolling X and Y, etc...)
    ; and the screen status (current raster position, v-blank and h-blank, etc...)
    DEFC VID_IO_CTRL_STAT = 0x90
        ; 16-bit values representing the current raster position (RO). Values latched when LSB read.
        DEFC IO_STAT_VPOS_LOW  = VID_IO_CTRL_STAT + 0x0  ; 16-bit value flushed when read
        DEFC IO_STAT_VPOS_HIGH = VID_IO_CTRL_STAT + 0x1
        DEFC IO_STAT_HPOS_LOW  = VID_IO_CTRL_STAT + 0x2  ; 16-bit value flushed when read
        DEFC IO_STAT_HPOS_HIGH = VID_IO_CTRL_STAT + 0x3
        ; 16-bit Y scrolling value for Layer0, in GFX mode (R/W). Value latched when MSB written.
        DEFC IO_CTRL_L0_SCR_Y_LOW  = VID_IO_CTRL_STAT + 0x4
        DEFC IO_CTRL_L0_SCR_Y_HIGH = VID_IO_CTRL_STAT + 0x5
        ; 16-bit X scrolling value for Layer0, in GFX mode (R/W). Value latched when MSB written.
        DEFC IO_CTRL_L0_SCR_X_LOW  = VID_IO_CTRL_STAT + 0x6
        DEFC IO_CTRL_L0_SCR_X_HIGH = VID_IO_CTRL_STAT + 0x7
        ; Similarly for Layer1 (R/W)
        DEFC IO_CTRL_L1_SCR_Y_LOW  = VID_IO_CTRL_STAT + 0x8
        DEFC IO_CTRL_L1_SCR_Y_HIGH = VID_IO_CTRL_STAT + 0x9
        DEFC IO_CTRL_L1_SCR_X_LOW  = VID_IO_CTRL_STAT + 0xa
        DEFC IO_CTRL_L1_SCR_X_HIGH = VID_IO_CTRL_STAT + 0xb
        ; Video mode register (R/W). Only takes effect after a V-blank occurs.
        DEFC IO_CTRL_VID_MODE      = VID_IO_CTRL_STAT + 0xc
        ; Video mode status
        ; Bit 0 - Set when in H-blank (RO)
        ; Bit 1 - Set when in V-blank (RO)
        ; Bit 2:6 - Reserved
        ; Bit 7 - Set to enable screen. Black screen when unset. (R/W)
        DEFC IO_CTRL_STATUS_REG    = VID_IO_CTRL_STAT + 0xd


    ; I/O modules that can be banked will appear at address 0xA0 on the I/O bus.
    DEFC VID_IO_BANKED_ADDR = 0xA0

    ; ----------------------------------------------------------------------- ;
    ; Banked Text Control module, usable in text mode (640x480 or 320x240)    ;
    ; ----------------------------------------------------------------------- ;
    DEFC BANK_IO_TEXT_NUM = 0

    DEFC IO_TEXT_PRINT_CHAR = VID_IO_BANKED_ADDR + 0x0 ; (WO)
    DEFC IO_TEXT_CURS_Y     = VID_IO_BANKED_ADDR + 0x1 ; (R/W) Cursor Y position (in characters count)
    DEFC IO_TEXT_CURS_X     = VID_IO_BANKED_ADDR + 0x2 ; (R/W) Cursor X position (in characters count)
    DEFC IO_TEXT_SCROLL_Y   = VID_IO_BANKED_ADDR + 0x3 ; (R/W) Scroll Y
    DEFC IO_TEXT_SCROLL_X   = VID_IO_BANKED_ADDR + 0x4 ; (R/W) Scroll X
    DEFC IO_TEXT_COLOR      = VID_IO_BANKED_ADDR + 0x5 ; (R/W) Current character color
    DEFC IO_TEXT_CURS_TIME  = VID_IO_BANKED_ADDR + 0x6 ; (R/W) Blink time, in frames, for the cursor
    DEFC IO_TEXT_CURS_CHAR  = VID_IO_BANKED_ADDR + 0x7 ; (R/W) Character from the font table for the cursor
    DEFC IO_TEXT_CURS_COLOR = VID_IO_BANKED_ADDR + 0x8 ; (R/W) Background and foreground colors for the cursor
    ; Control register, check the flags below to see what can be achieved
    DEFC IO_TEXT_CTRL_REG   = VID_IO_BANKED_ADDR + 0x9
        DEFC IO_TEXT_SAVE_CURSOR_BIT    = 7  ; Save the current cursor position (single save only)
        DEFC IO_TEXT_RESTORE_CURSOR_BIT = 6  ; Restore the previously saved position
        DEFC IO_TEXT_AUTO_SCROLL_X_BIT  = 5
        DEFC IO_TEXT_AUTO_SCROLL_Y_BIT  = 4
        ; When the cursor is about to wrap to the next line (maximum amount of characters sent
        ; to the screen), this flag can wait for the next character to come before resetting
        ; the cursor X position to 0 and potentially scroll the whole screen.
        ; Useful to implement an eat-newline fix.
        DEFC IO_TEXT_WAIT_ON_WRAP_BIT   = 3
        ; On READ, tells if the previous PRINT_CHAR (or NEWLINE) triggered a scroll in Y
        ; On WRITE, makes the cursor go to the next line
        DEFC IO_TEXT_SCROLL_Y_OCCURRED   = 0
        DEFC IO_TEXT_CURSOR_NEXTLINE     = 0


    ; ----------------------------------------------------------------------- ;
    ; Banked SPI Control module                                               ;
    ; ----------------------------------------------------------------------- ;
    DEFC BANK_IO_SPI_NUM = 1

    DEFC IO_SPI_VERSION     = VID_IO_BANKED_ADDR + 0x0 ; (RO)
    DEFC IO_SPI_CTRL        = VID_IO_BANKED_ADDR + 0x1
        DEFC IO_SPI_CTRL_START    = 7 ; (WO) Bit 7 - Start the SPI transaction
        DEFC IO_SPI_CTRL_RESET    = 6 ; (WO) Bit 6 - Reset the SPI controller
        DEFC IO_SPI_CTRL_CS_START = 5 ; (WO) Bit 5 - When set, selected CS will go low
        DEFC IO_SPI_CTRL_CS_STOP  = 4 ; (WO) Bit 4 - When set, selected CS will go high
        DEFC IO_SPI_CTRL_CS       = 3 ; (WO) Bit 3 - CS select. 0 for the SDCard, 1 is reserved
        DEFC IO_SPI_CTRL_RSVD1    = 2 ; Bit 2 - Reserved
        DEFC IO_SPI_CTRL_RSVD2    = 1 ; Bit 1 - Reserved
        DEFC IO_SPI_CTRL_IDLE     = 0 ; (RO) Bit 0 - Check if the SPI controller is in idle state

    DEFC IO_SPI_CLK_DIV     = VID_IO_BANKED_ADDR + 0x2 ; (R/W) Master clock divider, output frequency is 50/(2*div) MHz
    ; The SPI controller presents two 8-byte RAMs, one for incoming data and one for outgoing data. Since these arrays
    ; will always be filled with the same amount of bytes, they share the following register.
    ; These RAMs can be accessed as regular arrays or via an incremental indexes. This is convenient to send (or receive) data
    ; thanks to the `outi`/`otir` instructions (or `ini`/`inir` instructions respectively).
    ; The following register lets you specify the number of bytes  filled in the array.
    ; The MSB resets the incremental access indexes, any following write would store the received bytes starting at index 0.
    DEFC IO_SPI_RAM_LEN     = VID_IO_BANKED_ADDR + 0x3 ; (R/W)
        DEFC IO_SPI_RAM_CLEAR     = 7 ; Bit 7 - When set, resets the indexes for incremental write
        DEFC IO_SPI_RAM_BYTES     = 0 ; Bit 0:2 - Number of bytes to send from the array (starting at index 0)

    DEFC IO_SPI_RSVD4       = VID_IO_BANKED_ADDR + 0x4
    DEFC IO_SPI_RSVD5       = VID_IO_BANKED_ADDR + 0x5
    DEFC IO_SPI_RSVD6       = VID_IO_BANKED_ADDR + 0x6

    ; Read or write from and to the SPI RAM via a single register (address)
    ; When writing, it write to the SPI output array, when reading, it reads from the SPI input array.
    DEFC IO_SPI_RAM_FIFO    = VID_IO_BANKED_ADDR + 0x7 ; (R/W)
    DEFC IO_SPI_RAM         = VID_IO_BANKED_ADDR + 0x8 ; (R/W) Index 0 of the arrays

    DEFC IO_SPI_RAM_0       = IO_SPI_RAM               ; (R/W) Index 0 of the arrays
    DEFC IO_SPI_RAM_1       = VID_IO_BANKED_ADDR + 0x9 ; (R/W) Index 1 of the arrays
    DEFC IO_SPI_RAM_2       = VID_IO_BANKED_ADDR + 0xa ; (R/W) Index 2 of the arrays
    DEFC IO_SPI_RAM_3       = VID_IO_BANKED_ADDR + 0xb ; (R/W) Index 3 of the arrays
    DEFC IO_SPI_RAM_4       = VID_IO_BANKED_ADDR + 0xc ; (R/W) Index 4 of the arrays
    DEFC IO_SPI_RAM_5       = VID_IO_BANKED_ADDR + 0xd ; (R/W) Index 5 of the arrays
    DEFC IO_SPI_RAM_6       = VID_IO_BANKED_ADDR + 0xe ; (R/W) Index 6 of the arrays
    DEFC IO_SPI_RAM_7       = VID_IO_BANKED_ADDR + 0xf ; (R/W) Index 7 of the arrays

    ; ----------------------------------------------------------------------- ;
    ; CRC32 module                                                            ;
    ; ----------------------------------------------------------------------- ;
    DEFC BANK_IO_CRC_NUM = 2

    DEFC IO_CRC32_CTRL    = VID_IO_BANKED_ADDR + 0x0 ; (WO)
        DEFC IO_CRC32_CTRL_RESET = 0 ; Bit 0 - when set, resets the CRC32 controller

    DEFC IO_CRC32_DATA_IN = VID_IO_BANKED_ADDR + 0x1 ; (WO) Write data to this register, one byte after the other

    ; Resulting 32-bit CRC checksum (0x04C11DB7 Poly)
    DEFC IO_CRC32_BYTE0   = VID_IO_BANKED_ADDR + 0x4 ; (RO)
    DEFC IO_CRC32_BYTE1   = VID_IO_BANKED_ADDR + 0x5 ; (RO)
    DEFC IO_CRC32_BYTE2   = VID_IO_BANKED_ADDR + 0x6 ; (RO)
    DEFC IO_CRC32_BYTE3   = VID_IO_BANKED_ADDR + 0x7 ; (RO)

    ; ----------------------------------------------------------------------- ;
    ; Sound module                                                            ;
    ; ----------------------------------------------------------------------- ;
    DEFC BANK_IO_SOUND_NUM = 3

    ; There are a total of 4 voices on the Zeal 8-bit Video Board, each of them can generate triangle, square or sawtooth waves,
    ; they can also generate noise.
    ; The resulting frequency, in Hz can be calculated with the formula: (sample_rate * freq) / 2^16
    ; Where `sample_rate` is 44100 and `freq` is the 16-bit value defined below (register 0 and 1)
    DEFC IO_SOUND_VOICE_FREQ_LOW   = 0x0 ; (WO) Lowest byte of the 16-bit frequency. This will be taken into accoutn only when the HIGH byte is written
    DEFC IO_SOUND_VOICE_FREQ_HIGH  = 0x1 ; (WO) Highest byte of the 16-bit frequency, latches for the previously written lowest byte too.
    DEFC IO_SOUND_VOICE_WAVEFORM   = 0x2 ; (WO) Write the lowest 2 bits to choose the waveform
        DEFC IO_SOUND_SQUARE_WAVE   = 0
        DEFC IO_SOUND_TRIANGLE_WAVE = 1
        DEFC IO_SOUND_SAWTOOTH_WAVE = 2
        DEFC IO_SOUND_NOISE         = 3

    DEFC IO_SOUND_HOLD       = VID_IO_BANKED_ADDR + 0xD ; (WO) Bitmap where bit i hold voice i (1 = on hold, 0 = can outptu sound).
                                                  ; This is convenient when you need two voices to start at the exact same time.
    ; The register 0xE is a bitmap that selects/deselects the voice that will listen to the configuration registers.
    ; It means that if reg[0xE] contains 0x0F, all 4 voices will listen to the writes performed on the configuration registers.
    DEFC IO_SOUND_MASTER_VOL = VID_IO_BANKED_ADDR + 0xE ; (WO) Master volume
        DEFC IO_SOUND_VOL_DISABLE = 7 ; (WO) Disable master volume (volume = 0)
        DEFC IO_SOUND_VOL_LEVEL   = 0 ; (WO) Bit 0-1, 0b00 = 25%, 0b01 = 50%, 0b10 = 75%, 0b11 = 100%
    DEFC IO_SOUND_SEL_VOICES = VID_IO_BANKED_ADDR + 0xF ; (WO) Bitmap where bit i selects voice i to listen on the registers [0:7] (1 = selected, 0 = deselected)


    ; Video modes that can be given to IO_CTRL_VID_MODE register
    DEFC VID_MODE_TEXT_640     = 0
    DEFC VID_MODE_TEXT_320     = 1
    DEFC VID_MODE_GFX_640_8BIT = 4
    DEFC VID_MODE_GFX_320_8BIT = 5
    DEFC VID_MODE_GFX_640_4BIT = 6
    DEFC VID_MODE_GFX_320_4BIT = 7

    ; Macros for text-mode
    DEFC VID_640480_WIDTH = 640
    DEFC VID_640480_HEIGHT = 480
    DEFC VID_640480_X_MAX = 80
    DEFC VID_640480_Y_MAX = 40
    DEFC VID_640480_TOTAL = VID_640480_X_MAX * VID_640480_Y_MAX

    ENDIF
