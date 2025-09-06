cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
set(ZVB_SDK_DIR "$ENV{ZVB_SDK_PATH}/cmake")

project(timer_example C)

add_executable(timer "src/main.c")

find_package(ZVB_SDK REQUIRED)
target_link_libraries(timer PUBLIC zvb_sound)

zos_add_outputs(timer)
