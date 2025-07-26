/**
 * SPDX-FileCopyrightText: 2024-2025 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @brief The SDK uses a versioning scheme in the format MAJ.MIN.REV.VER, where:
 * - MAJ.MIN.REV corresponds to the supported ZVB firmware version.
 * - VER is the SDK’s own release number for that firmware version.
 *
 * For example, sdk-0.1.0.0 and sdk-0.1.0.99 are both compatible with ZVB firmware version 0.1.0.
 * However, sdk-0.1.0.99 is a newer SDK release and may include additional features or bug fixes.
 */
#define ZVB_SDK_MAJOR   1
#define ZVB_SDK_MINOR   0
#define ZVB_SDK_REV     0
#define ZVB_SDK_VER     0