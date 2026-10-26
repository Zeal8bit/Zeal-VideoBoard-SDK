cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(flag C ASM)

add_executable(flag
    src/flag.c
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(flag PRIVATE zvb_gfx)

zos_add_outputs(flag)
