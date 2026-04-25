; SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
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

_zvb_config_dev_idx	=	0x008e
_zvb_peri_dma_ctrl	=	0x00a0
_zvb_peri_dma_addr0	=	0x00a1
_zvb_peri_dma_addr1	=	0x00a2
_zvb_peri_dma_addr2	=	0x00a3

	.area _TEXT

; Internal helper used after callers have normalized arguments into registers.
;
; Input:
;   HL    = zvb_dma_descriptor_t* desc
;   E:D:C = 24-bit physical read address, low byte first
; Output:
;   A = 0 (ERR_SUCCESS)
; Clobbers:
;   HL, A
;
; Descriptor layout written here:
;   desc + 0 = rd_addr_lo low
;   desc + 1 = rd_addr_lo high
;   desc + 2 = rd_addr_hi
zvb_dma_set_read_regs:
	; Store address byte 0.
	ld	(hl), e
	inc	hl

	; Store address byte 1.
	ld	(hl), d
	inc	hl

	; Store address byte 2.
	ld	(hl), c

	; Return ERR_SUCCESS.
	xor	a, a
	ret

; Internal helper used after callers have normalized arguments into registers.
;
; Input:
;   HL    = zvb_dma_descriptor_t* desc
;   E:D:C = 24-bit physical write address, low byte first
; Output:
;   A = 0 (ERR_SUCCESS)
; Clobbers:
;   HL, A
;
; Descriptor layout written here:
;   desc + 3 = wr_addr_lo low
;   desc + 4 = wr_addr_lo high
;   desc + 5 = wr_addr_hi
zvb_dma_set_write_regs:
	; Advance HL from desc base to wr_addr_lo.
	inc	hl
	inc	hl
	inc	hl

	; Store address byte 0.
	ld	(hl), e
	inc	hl

	; Store address byte 1.
	ld	(hl), d
	inc	hl

	; Store address byte 2.
	ld	(hl), c

	; Return ERR_SUCCESS.
	xor	a, a
	ret

; uint32_t zvb_dma_virt_to_phys(void* ptr)
;
; SDCC calling convention:
;   HL = ptr
;
; Return:
;   HLDE = 32-bit physical address
;   E    = address byte 0
;   D    = address byte 1
;   L    = address byte 2
;   H    = 0
;
; The Zeal memory mapper exposes the current physical page through port 0xf0.
; The pointer's low 14 bits come from HL; the upper physical bits come from the
; active page register.
_zvb_dma_virt_to_phys::
	; Keep pointer low byte as physical address byte 0.
	ld	e, l

	; Keep pointer bits 8..13 as the low six bits of physical address byte 1.
	ld	a, h
	and	#0x3f
	ld	d, a

	; Read current page register and rotate its page bits into position.
	ld	a, h
	in	a, (#0xf0)
	rrca
	rrca
	ld	h, a

	; Merge physical page bits 14..15 into address byte 1.
	and	#0xc0
	or	d
	ld	d, a

	; Put physical page bits 16..21 into address byte 2.
	ld	a, h
	and	#0x3f
	ld	l, a

	; Highest byte is always zero for the 24-bit DMA address.
	ld	h, #0x00
	ret

; uint8_t zvb_dma_prepare_descriptor(
;     zvb_dma_descriptor_t* desc,
;     zvb_dma_descriptor_config_t* config
; )
;
; SDCC calling convention:
;   HL = desc
;   DE = config
;
; Source config layout:
;   config + 0  = rd_addr byte 0
;   config + 1  = rd_addr byte 1
;   config + 2  = rd_addr byte 2
;   config + 3  = rd_addr byte 3, unused by DMA descriptor
;   config + 4  = wr_addr byte 0
;   config + 5  = wr_addr byte 1
;   config + 6  = wr_addr byte 2
;   config + 7  = wr_addr byte 3, unused by DMA descriptor
;   config + 8  = length low
;   config + 9  = length high
;   config + 10 = flags
;
; Destination descriptor layout:
;   desc + 0 = rd_addr_lo low
;   desc + 1 = rd_addr_lo high
;   desc + 2 = rd_addr_hi
;   desc + 3 = wr_addr_lo low
;   desc + 4 = wr_addr_lo high
;   desc + 5 = wr_addr_hi
;   desc + 6 = length low
;   desc + 7 = length high
;   desc + 8 = flags
_zvb_dma_prepare_descriptor::
	; Copy rd_addr byte 0.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy rd_addr byte 1.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy rd_addr byte 2.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Skip rd_addr byte 3; DMA descriptor stores only 24 bits.
	inc	de

	; Copy wr_addr byte 0.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy wr_addr byte 1.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy wr_addr byte 2.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Skip wr_addr byte 3; DMA descriptor stores only 24 bits.
	inc	de

	; Copy length low byte.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy length high byte.
	ld	a, (de)
	ld	(hl), a
	inc	de
	inc	hl

	; Copy flags byte.
	ld	a, (de)
	ld	(hl), a

	; Return ERR_SUCCESS.
	xor	a, a
	ret

; uint8_t zvb_dma_set_read(zvb_dma_descriptor_t* desc, uint32_t addr)
;
; SDCC calling convention:
;   HL = desc
;   Stack on entry:
;     SP + 0 = return address
;     SP + 2 = addr byte 0
;     SP + 3 = addr byte 1
;     SP + 4 = addr byte 2
;     SP + 5 = addr byte 3
;
; This public ABI receives the 32-bit addr on the stack. Normalize it to the
; internal helper contract:
;   HL    = desc
;   E:D:C = addr byte 0, byte 1, byte 2
;
; The high address byte is popped into B and ignored. Re-pushing AF restores the
; return address so the shared helper can return normally.
_zvb_dma_set_read::
	pop	af
	pop	de
	pop	bc
	push	af
	jp	zvb_dma_set_read_regs

; uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t* desc, void* ptr)
;
; SDCC calling convention:
;   HL = desc
;   DE = ptr
;
; Convert ptr to physical address, then tail-jump to the register helper.
; _zvb_dma_virt_to_phys returns the useful DMA bytes in E, D, and L.
_zvb_dma_set_read_virt::
	; Put ptr in HL for _zvb_dma_virt_to_phys and save desc on stack.
	ex	de, hl
	push	de
	call	_zvb_dma_virt_to_phys

	; Move returned address byte 2 from L to C for helper contract.
	ld	c, l

	; Restore desc into HL.
	pop	hl
	jp	zvb_dma_set_read_regs

; uint8_t zvb_dma_set_write(zvb_dma_descriptor_t* desc, uint32_t addr)
;
; SDCC calling convention:
;   HL = desc
;   Stack on entry:
;     SP + 0 = return address
;     SP + 2 = addr byte 0
;     SP + 3 = addr byte 1
;     SP + 4 = addr byte 2
;     SP + 5 = addr byte 3
;
; Normalize the public stack ABI to the internal helper contract:
;   HL    = desc
;   E:D:C = addr byte 0, byte 1, byte 2
_zvb_dma_set_write::
	pop	af
	pop	de
	pop	bc
	push	af
	jp	zvb_dma_set_write_regs

; uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t* desc, void* ptr)
;
; SDCC calling convention:
;   HL = desc
;   DE = ptr
;
; Convert ptr to physical address, then tail-jump to the register helper.
_zvb_dma_set_write_virt::
	; Put ptr in HL for _zvb_dma_virt_to_phys and save desc on stack.
	ex	de, hl
	push	de
	call	_zvb_dma_virt_to_phys

	; Move returned address byte 2 from L to C for helper contract.
	ld	c, l

	; Restore desc into HL.
	pop	hl
	jp	zvb_dma_set_write_regs

; uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t* desc)
;
; SDCC calling convention:
;   HL = desc
;
; Convert descriptor pointer to physical address, map DMA peripheral registers,
; write the 24-bit descriptor address, then start the transfer.
_zvb_dma_start_transfer::
	call	_zvb_dma_virt_to_phys

	; Map peripheral window 0xa0..0xaf to the DMA device.
	ld	a, #0x04
	out	(_zvb_config_dev_idx), a

	; Write descriptor physical address bytes 0..2.
	ld	a, e
	out	(_zvb_peri_dma_addr0), a
	ld	a, d
	out	(_zvb_peri_dma_addr1), a
	ld	a, l
	out	(_zvb_peri_dma_addr2), a

	; Start transfer.
	ld	a, #0x80
	out	(_zvb_peri_dma_ctrl), a

	; Return ERR_SUCCESS.
	xor	a, a
	ret
