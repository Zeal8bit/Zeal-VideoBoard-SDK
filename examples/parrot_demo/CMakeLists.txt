cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN z88dk)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

project(parrot_demo ASM)

if(NOT DEFINED ENV{ZVB_SDK_PATH})
    message(FATAL_ERROR "Set ZVB_SDK_PATH to the Zeal Video Board SDK directory")
endif()

add_executable(parrot src/parrot.asm)
target_include_directories(parrot PRIVATE
    "$ENV{ZVB_SDK_PATH}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_options(parrot PRIVATE -m)
set_property(SOURCE src/parrot.asm APPEND PROPERTY OBJECT_DEPENDS
    "$ENV{ZVB_SDK_PATH}/include/zvb_hardware_h.asm"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/parrot.ztp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/parrot.ztm"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/parrot.zts"
)

find_program(BASH_EXECUTABLE bash)
if(NOT BASH_EXECUTABLE)
    message(FATAL_ERROR "Bash is required to combine the Parrot flash image")
endif()
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/parrot.bin.img"
    COMMAND "${BASH_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/concat.sh"
        "${CMAKE_CURRENT_BINARY_DIR}/parrot.bin.img"
        0x0000 "$<TARGET_FILE:parrot>"
        0x4000 "${CMAKE_CURRENT_BINARY_DIR}/parrot_TILESET.bin"
    DEPENDS parrot "${CMAKE_CURRENT_SOURCE_DIR}/concat.sh"
    COMMENT "Combining Parrot code and tileset into a flash image"
    VERBATIM
)
add_custom_target(parrot_image ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/parrot.bin.img")
