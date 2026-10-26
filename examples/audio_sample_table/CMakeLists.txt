cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(audio_sample_table C ASM)

add_executable(audio
    src/main.c
)

zos_add_assets(audio
    hello_raw_audio.bin
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(audio PRIVATE zvb_sound)

zos_add_outputs(audio)
