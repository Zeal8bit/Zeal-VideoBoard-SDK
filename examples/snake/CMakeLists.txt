cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
include($ENV{ZVB_SDK_PATH}/cmake/zvb_init.cmake)

project(snake_game C ASM)

add_executable(snake "src/snake.c"
                     "src/controller.c"
                     "src/title.c")

target_compile_definitions(snake PUBLIC GENERATED_ASSETS=1)

zos_add_assets(snake "assets/snake_tileset.zts"
                     "assets/letters.zts"
                     "assets/numbers.zts")

find_package(ZVB_SDK REQUIRED)
target_link_libraries(snake PUBLIC zvb_gfx)

zos_add_outputs(snake)
