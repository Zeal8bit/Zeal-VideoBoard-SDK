/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_hardware.h"


/**
 * @brief Initialize the CRC peripheral.
 *
 * @note After calling this function, the peripheral will be mapped in the peripheral bank.
 *
 * @param reset When set, the CRC calculation will be reset.
 */
void zvb_crc_initialize(uint8_t reset);


/**
 * @brief Reset the CRC calculation.
 */
void zvb_crc_reset(void);


/**
 * @brief Update the CRC calculation with the given buffer.
 *
 * @param buffer Buffer containing the bytes to feed to the CRC calculation (must NOT be NULL)
 * @param size Number of bytes in the buffer.
 *
 * @return CRC32 result
 */
uint32_t zvb_crc_update(uint8_t *buffer, uint16_t size)  __sdcccall(1);
