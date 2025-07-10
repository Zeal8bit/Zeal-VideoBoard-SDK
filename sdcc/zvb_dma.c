#include <stdint.h>
#include "zvb_hardware.h"
#include "zvb_dma.h"

uint32_t zvb_dma_virt_to_phys(void* ptr) __naked {
    (void*)ptr;
    __asm__ (
        // Prepare the lowest bits
        "ld e, l\n"
        "ld a, h\n"
        "and #0x3f\n"
        "ld d, a\n"
        // Get the page value
        "ld a, h\n"
        "in a, (#0xF0)\n"
        "rrca\n"
        "rrca\n"
        // Put the high bits in D
        "ld h, a\n"
        "and #0xc0\n"
        "or d\n"
        "ld d, a\n"
        // Put the lowest bits in L
        "ld a, h\n"
        "and #0x3f\n"
        "ld l, a\n"
        "ld h, #0\n"
        "ret\n"
    );
}

uint8_t zvb_dma_prepare_descriptor(zvb_dma_descriptor_t* desc, zvb_dma_descriptor_config_t* config) {
    desc->rd_addr_lo = config->rd_addr & 0xFFFF;
    desc->rd_addr_hi = (config->rd_addr >> 16) & 0xFF;
    desc->wr_addr_lo = config->wr_addr & 0xFFFF;
    desc->wr_addr_hi = (config->wr_addr >> 16) & 0xFF;
    desc->length = config->length;
    desc->flags = config->flags;

    return 0; // ERR_SUCCESS
}

uint8_t zvb_dma_set_read(zvb_dma_descriptor_t* desc, uint32_t addr) {
    desc->rd_addr_lo = addr & 0xFFFF;
    desc->rd_addr_hi = (addr >> 16) & 0xFF;

    return 0; // ERR_SUCCESS
}

uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t* desc, void* ptr) {
    uint32_t addr = zvb_dma_virt_to_phys(ptr);
    return zvb_dma_set_read(desc, addr);
}

uint8_t zvb_dma_set_write(zvb_dma_descriptor_t* desc, uint32_t addr) {
    desc->wr_addr_lo = addr & 0xFFFF;
    desc->wr_addr_hi = (addr >> 16) & 0xFF;

    return 0; // ERR_SUCCESS
}

uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t* desc, void* ptr) {
    uint32_t addr = zvb_dma_virt_to_phys(ptr);
    return zvb_dma_set_write(desc, addr);
}

uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t *desc) {
    uint32_t desc_phys_addr = zvb_dma_virt_to_phys(desc);

    zvb_map_peripheral(ZVB_PERI_DMA_IDX);
    zvb_peri_dma_addr0 = (desc_phys_addr >> 0) & 0xFF;
    zvb_peri_dma_addr1 = (desc_phys_addr >> 8) & 0xFF;
    zvb_peri_dma_addr2 = (desc_phys_addr >> 16) & 0xFF;
    zvb_peri_dma_ctrl = ZVB_PERI_DMA_CTRL_START;

    return 0; // ERR_SUCCESS
}
