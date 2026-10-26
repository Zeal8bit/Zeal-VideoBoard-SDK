cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(sound C ASM)

add_executable(play
    src/main.c
    src/parser.c
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(play PRIVATE zvb_sound)

zos_add_outputs(play)
