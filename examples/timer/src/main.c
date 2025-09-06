/**
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zvb_hardware.h>

#define COUNTER  1000

IOB(0xd1) pio_data_b;
IOB(0xd3) pio_ctrl_b;


static void timer_isr(void) __naked
{
__asm
    push af
    ld a, #ZVB_PERI_TIMER_IDX
    out (_zvb_config_dev_idx), a
    ld a, #1
    out (_zvb_peri_time_int_st), a
    ld a, #ZVB_PERI_TEXT_IDX
    out (_zvb_config_dev_idx), a
    ld a, #'T'
    out (_zvb_peri_text_print_char), a
    ld a, #' '
    out (_zvb_peri_text_print_char), a
    pop af
    ei
    reti
__endasm;
    /* Acknowledge the interrupt */
    // zvb_map_peripheral(ZVB_PERI_TIMER_IDX);
    // zvb_peri_time_int_st = 1;
    // zvb_map_peripheral(ZVB_PERI_TEXT_IDX);
    // zvb_peri_text_print_char = 'T';
    // zvb_peri_text_print_char = ' ';
    // printf("%x:%x:%x\n",
    //     zvb_ctrl_int_peri,
    //     zvb_ctrl_int_st_clr,
    //     pio_data_b
    // );
}

static __at(0x8000) void* isr_vector[2];


static void set_interrupt_vector(void) {
    isr_vector[1] = timer_isr;
    __asm__ ("ld a, #0x80\n");
    __asm__ ("ld i, a\n");
}


int main(int argc, char** argv)
{
#if 0
    while (1) {
        printf("PIO state: %x, st: %x, peri: %x, int_st: %x\n",
            pio_data_b,
            zvb_ctrl_status,
            zvb_ctrl_int_peri,
            zvb_ctrl_int_st_clr);
        msleep(500);
    }
#endif
    __asm__("di");

    /* Enable interrupts on the PIO */
    pio_ctrl_b = 0x97;
    pio_ctrl_b = (uint8_t) (~(1 << 5));
    set_interrupt_vector();

    /* Map the timer controller */
    zvb_map_peripheral(ZVB_PERI_TIMER_IDX);

    /* Let's make a timer that prints a message every 1s.
     * Configure each ticks period to 1ms, and the counter to 1000, counting down
     * Divider = 1ms * 50MHz - 1 = 49,999
     */
    zvb_peri_time_div_lo = (49999 & 0xff);
    zvb_peri_time_div_hi = (49999 >> 8) & 0xff;

    zvb_peri_time_rel_lo = COUNTER & 0xff;
    zvb_peri_time_rel_hi = (COUNTER >> 8) & 0xff;

    zvb_peri_time_cnt_lo = COUNTER & 0xff;
    zvb_peri_time_cnt_hi = (COUNTER >> 8) & 0xff;

    /* Start the timer and schedule interrupts */
    zvb_peri_timer_ctrl = ZVB_PERI_TIMER_CTRL_ENABLE  |
                          ZVB_PERI_TIMER_CTRL_AUTOREL |
                          ZVB_PERI_TIMER_CTRL_DIR     | // Down
                          ZVB_PERI_TIMER_CTRL_INT_ENA;

    __asm__("ei");

    while(1) {
    }

    uint8_t i = 0;
    while (1) {
        zvb_map_peripheral(ZVB_PERI_TIMER_IDX);
        if (zvb_peri_time_int_st) {
            /* Get the global interrupt registers */
            uint8_t peri = zvb_ctrl_int_peri;
            uint8_t st   = zvb_ctrl_int_st_clr;
            uint8_t pio  = pio_data_b;
            /* Acknowledge the interrupt */
            zvb_peri_time_int_st = 1;
            printf("%d: %x:%x:%x -> %x:%x:%x\n", i++,
                peri,
                st,
                pio,

                zvb_ctrl_int_peri,
                zvb_ctrl_int_st_clr,
                pio_data_b
            );
        }
    }

    return 0;
}

