/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "zvb_hardware.h"

#define SOUND_FREQ_TO_DIV(FREQ)     (65536*(FREQ) / 44091)
#define SOUND_SAMPLE_TABLE_SIZE     256

typedef enum {
    VOICE0 = 1 << 0,
    VOICE1 = 1 << 1,
    VOICE2 = 1 << 2,
    VOICE3 = 1 << 3,
    SAMPTAB = 1 << 7,
} sound_voice_t;


/**
 * @brief Define a type for the macros that are part of the hardware header
 */
typedef enum {
    WAV_SQUARE    = ZVB_PERI_SOUND_SQUARE,
    WAV_TRIANGLE  = ZVB_PERI_SOUND_TRIANGLE,
    WAV_SAWTOOTH  = ZVB_PERI_SOUND_SAWTOOTH,
    WAV_NOISE     = ZVB_PERI_SOUND_NOISE,
} sound_waveform_t;

/**
 * @brief Define a type for the volume, in percentage
 */
typedef enum {
    VOL_0   = ZVB_PERI_SOUND_VOL_DISABLE,
    VOL_25  = ZVB_PERI_SOUND_VOL_25,
    VOL_50  = ZVB_PERI_SOUND_VOL_50,
    VOL_75  = ZVB_PERI_SOUND_VOL_75,
    VOL_100 = ZVB_PERI_SOUND_VOL_100,
} sound_volume_t;

// Duty Cycle / PWM (for WAV_SQUARE)
typedef enum {
    DUTY_CYCLE_12_5 = 0b001 << 5, // 12.5
    DUTY_CYCLE_25_0 = 0b010 << 5, // 25
    DUTY_CYCLE_37_5 = 0b011 << 5, // 25 + 12.5
    DUTY_CYCLE_50_0 = 0b100 << 5, // 50
    DUTY_CYCLE_62_5 = 0b101 << 5, // 50 + 12.5
    DUTY_CYCLE_75_0 = 0b110 << 5, // 50 + 25
    DUTY_CYCLE_87_5 = 0b111 << 5, // 50 + 25 + 12.5
} sound_duty_cycle_t;


typedef enum {
    SAMPLE_UINT8,   // Unsigned 8-bit samples
    SAMPLE_UINT16,  // Unsigned 16-bit samples
    SAMPLE_SINT16,  // Signed 16-bit samples
} sample_int;


/**
 * @brief Configuration for the sample table voice
 */
 typedef struct {
    sample_int mode;    // When set, the samples are 8-bit unsigned values
    uint8_t divider;    // Sample rate divider
 } sound_samples_conf_t;


/**
 * @brief Initialize the sound peripheral.
 *
 * @note After calling this function, the peripheral will be mapped in the peripheral bank.
 *
 * @param reset When set, the sound voices will be reset.
 */
void zvb_sound_initialize(uint8_t reset);


/**
 * @brief Reset all the sound voices.
 */
void zvb_sound_reset(void);


/**
 * @brief Set one or multiple voices outputs.
 *
 * @param voices Voices to set, the VOICE[0-3] values can be ORed.
 *        SAMPTAB must NOT be specified.
 */
void zvb_sound_set_voices(sound_voice_t voices, uint16_t divider, sound_waveform_t waveform);


/**
 * @brief Set volume for one or multiple voices.
 *
 * @param voices Voices to set, the VOICE[0-3] values can be ORed.
 *        SAMPTAB must NOT be specified.
 */
void zvb_sound_set_voices_vol(sound_voice_t voices, sound_volume_t vol);


/**
 * @brief Hold (mute) or unhold (start) the given voice(s) output
 */
void zvb_sound_set_hold(sound_voice_t voices, uint8_t hold);


/**
 * @brief Assign voices to the channels.
 *
 * @param left_voices  Voices to set, SAMPTAB and VOICE[0-3] values can be ORed.
 * @param right_voices Voices to set, SAMPTAB and VOICE[0-3] values can be ORed.
 */
void zvb_sound_set_channels(sound_voice_t left_voices, sound_voice_t right_voices);


/**
 * @brief Set the master volume for both channels.
 */
void zvb_sound_set_volume(sound_volume_t vol);


/**
 * @brief Set the master volume for the left and right channels respectively.
 */
void zvb_sound_set_volumes(sound_volume_t left, sound_volume_t right);


/**
 * @brief Play samples on the sample table voice
 *
 *
 * @param div Sample rate divider to play the samples at. The resulting sample rate
               is 44100/div
 * @param samples Table containing the samples to play
 * @param length Length of the table
 */
void zvb_sound_play_samples(sound_samples_conf_t* config, void* samples, uint16_t length);
