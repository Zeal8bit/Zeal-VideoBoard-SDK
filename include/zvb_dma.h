/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_hardware.h"

#define DMA_DESC_LAST 1

typedef union {
    struct {
        uint8_t rd_cycle : 4;
        uint8_t wr_cycle : 4;
    };
    uint8_t raw;
} zvb_dma_clk_t;

typedef union {
    struct {
        uint8_t last  : 1;
        uint8_t rd_op : 2;
        uint8_t wr_op : 2;
        uint8_t rsd   : 3;
    };
    uint8_t raw;
} dma_flags_t;

typedef struct {
    uint16_t rd_addr_lo;  // Source address
    uint8_t rd_addr_hi;   // Source address
    uint16_t wr_addr_lo;  // Destination address
    uint8_t wr_addr_hi;   // Destination address
    uint16_t length;      // Transfer length in bytes
    dma_flags_t flags;    // Descriptor flags
    uint8_t reserved[3];  // Padding/future use
} zvb_dma_descriptor_t;

typedef struct {
    uint32_t rd_addr;
    uint32_t wr_addr;
    uint16_t length;
    dma_flags_t flags;
} zvb_dma_descriptor_config_t;


uint32_t zvb_dma_virt_to_phys(void* ptr) __naked;
uint8_t zvb_dma_prepare_descriptor(zvb_dma_descriptor_t* desc, zvb_dma_descriptor_config_t* config);
uint8_t zvb_dma_set_read(zvb_dma_descriptor_t* desc, uint32_t addr);
uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t* desc, void* ptr);
uint8_t zvb_dma_set_write(zvb_dma_descriptor_t* desc, uint32_t addr);
uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t* desc, void* ptr);
uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t *desc);
