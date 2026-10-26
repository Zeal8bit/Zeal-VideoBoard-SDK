cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(tilemap C ASM)

add_executable(tilemap
    src/tilemap.c
)

zos_add_assets(tilemap
    assets/tilemap.ztp
    assets/tilemap.zts
    assets/tilemap.ztm
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(tilemap PRIVATE zvb_gfx)

zos_add_outputs(tilemap)
