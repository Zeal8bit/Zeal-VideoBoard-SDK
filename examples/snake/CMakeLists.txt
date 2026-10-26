cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)

project(snake C ASM)

add_executable(snake
    src/snake.c
    src/controller.c
    src/title.c
)

zos_add_assets(snake
    assets/snake_tileset.zts
    assets/letters.zts
    assets/numbers.zts
)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(snake PRIVATE zvb_gfx)

zos_add_outputs(snake)
