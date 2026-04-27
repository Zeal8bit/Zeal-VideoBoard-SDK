; SPDX-FileCopyrightText: 2024-2026 Zeal 8-bit Computer <contact@zeal8bit.com>
;
; SPDX-License-Identifier: Apache-2.0

    .module zvb_dma
    .optsdcc -mz80

    .globl _zvb_dma_virt_to_phys
    .globl _zvb_dma_prepare_descriptor
    .globl _zvb_dma_set_read
    .globl _zvb_dma_set_read_virt
    .globl _zvb_dma_set_write
    .globl _zvb_dma_set_write_virt
    .globl _zvb_dma_start_transfer

_zvb_config_dev_idx = 0x8e
_zvb_peri_dma_ctrl  = 0xa0
_zvb_peri_dma_addr0 = 0xa1
_zvb_peri_dma_addr1 = 0xa2
_zvb_peri_dma_addr2 = 0xa3

    .area _TEXT



; Internal helper used after callers have normalized arguments into registers.
;
; Parameters:
;   HL  - zvb_dma_descriptor_t* desc
;   CDE - 24-bit physical write address, low byte first
; Returns:
;   A - 0 (ERR_SUCCESS)
;
; Descriptor layout written here:
;   desc + 3 = wr_addr_lo low
;   desc + 4 = wr_addr_lo high
;   desc + 5 = wr_addr_hi
zvb_dma_set_write_regs:
    ; Advance HL from desc base to wr_addr_lo.
    inc hl
    inc hl
    inc hl
    ; FALLTHROUGH zvb_dma_set_read_regs

; Internal helper used after callers have normalized arguments into registers.
;
; Parameters:
;   HL  - zvb_dma_descriptor_t* desc
;   CDE - 24-bit physical read address (E is LSB, C is MSB)
; Returns:
;   A = 0 (ERR_SUCCESS)
;
; Descriptor layout written here:
;   desc + 0 = rd_addr_lo low
;   desc + 1 = rd_addr_lo high
;   desc + 2 = rd_addr_hi
zvb_dma_set_read_regs:
    ; Store address byte 0.
    ld (hl), e
    inc hl

    ; Store address byte 1.
    ld (hl), d
    inc hl

    ; Store address byte 2.
    ld (hl), c

    ; Return ERR_SUCCESS.
    xor a
    ret


; uint32_t zvb_dma_virt_to_phys(void* ptr)
;
; Convert a virtual pointer to the 24-bit physical address used by the DMA
; controller.
;
; Parameters:
;   HL - Virtual address to convert
;
; Returns:
;   HLDE - 32-bit physical address (E is LSB, H is MSB)
_zvb_dma_virt_to_phys::
    ; Keep pointer low byte as physical address byte 0.
    ld e, l

    ; Keep pointer bits 8..13 as the low six bits of physical address byte 1.
    ld a, h
    and #0x3f
    ld d, a

    ; Read current page register and rotate its page bits into position.
    ld a, h
    in a, (#0xf0)
    rrca
    rrca
    ld h, a

    ; Merge physical page bits 14..15 into address byte 1.
    and #0xc0
    or d
    ld d, a

    ; Put physical page bits 16..21 into address byte 2.
    ld a, h
    and #0x3f
    ld l, a

    ; Highest byte is always zero for the 24-bit DMA address.
    ld h, #0x00
    ret


; uint8_t zvb_dma_prepare_descriptor(zvb_dma_descriptor_t* desc,
;                                    zvb_dma_descriptor_config_t* config)
;
; Populate a DMA descriptor from a descriptor configuration structure. Only the
; low 24 bits of the read and write addresses are stored in the descriptor.
;
; Parameters:
;   HL - DMA descriptor to fill
;   DE - Descriptor configuration containing addresses, length, and flags
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_prepare_descriptor::
    ; Put the configuration in HL and the descriptor to fill in DE
    ex de, hl
    ; Copy rd_addr byte 0.
    ldi
    ; Copy rd_addr byte 1.
    ldi
    ; Copy rd_addr byte 2.
    ldi
    ; Skip rd_addr byte 3; DMA descriptor stores only 24 bits.
    inc hl

    ; Copy wr_addr byte 0.
    ldi
    ; Copy wr_addr byte 1.
    ldi
    ; Copy wr_addr byte 2.
    ldi
    ; Skip wr_addr byte 3; DMA descriptor stores only 24 bits.
    inc hl

    ; Copy length low byte.
    ldi
    ; Copy length high byte.
    ldi
    ; Copy flags byte.
    ldi

    ; Return ERR_SUCCESS.
    xor a
    ret


; uint8_t zvb_dma_set_read(zvb_dma_descriptor_t* desc, uint32_t addr)
;
; Set the read address of a DMA descriptor from a physical address. Only the
; low 24 bits of the address are stored in the descriptor.
;
; Parameters:
;   HL - DMA descriptor to update
;   Stack - 32-bit physical address
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_set_read::
    pop af
    pop de
    pop bc
    push af
    jp zvb_dma_set_read_regs


; uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t* desc, void* ptr)
;
; Convert a virtual pointer to a physical address and store it as the read
; address of a DMA descriptor.
;
; Parameters:
;   HL - DMA descriptor to update
;   DE - Virtual address to convert and store
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_set_read_virt::
    ; Put ptr in HL for _zvb_dma_virt_to_phys and save desc on stack.
    ex de, hl
    push de
    call _zvb_dma_virt_to_phys

    ; Move returned address byte 2 from L to C for helper contract.
    ld c, l

    ; Restore desc into HL.
    pop hl
    jp zvb_dma_set_read_regs


; uint8_t zvb_dma_set_write(zvb_dma_descriptor_t* desc, uint32_t addr)
;
; Set the write address of a DMA descriptor from a physical address. Only the
; low 24 bits of the address are stored in the descriptor.
;
; Parameters:
;   HL - DMA descriptor to update
;   Stack - 32-bit physical address
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_set_write::
    pop af
    pop de
    pop bc
    push af
    jp zvb_dma_set_write_regs


; uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t* desc, void* ptr)
;
; Convert a virtual pointer to a physical address and store it as the write
; address of a DMA descriptor.
;
; Parameters:
;   HL - DMA descriptor to update
;   DE - Virtual address to convert and store
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_set_write_virt::
    ; Put ptr in HL for _zvb_dma_virt_to_phys and save desc on stack.
    ex de, hl
    push de
    call _zvb_dma_virt_to_phys

    ; Move returned address byte 2 from L to C for helper contract.
    ld c, l

    ; Restore desc into HL.
    pop hl
    jp zvb_dma_set_write_regs


; uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t* desc)
;
; Start a DMA transfer using the given descriptor.
;
; Parameters:
;   HL - DMA descriptor describing the transfer
;
; Returns:
;   A - ERR_SUCCESS
_zvb_dma_start_transfer::
    call _zvb_dma_virt_to_phys

    ; Map peripheral window 0xa0..0xaf to the DMA device.
    ld a, #0x04
    out (_zvb_config_dev_idx), a

    ; Write descriptor physical address bytes 0..2.
    ld a, e
    out (_zvb_peri_dma_addr0), a
    ld a, d
    out (_zvb_peri_dma_addr1), a
    ld a, l
    out (_zvb_peri_dma_addr2), a

    ; Start transfer.
    ld a, #0x80
    out (_zvb_peri_dma_ctrl), a

    ; Return ERR_SUCCESS.
    xor a, a
    ret
