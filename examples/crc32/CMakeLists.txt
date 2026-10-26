cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(crc32 C ASM)

add_executable(crc32
    src/crc32.c
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(crc32 PRIVATE zvb_crc)

zos_add_outputs(crc32)
