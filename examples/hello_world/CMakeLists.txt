cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN z88dk)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

project(hello_world ASM)

if(NOT DEFINED ENV{ZVB_SDK_PATH})
    message(FATAL_ERROR "Set ZVB_SDK_PATH to the Zeal Video Board SDK directory")
endif()

add_executable(hello src/hello.asm)
target_include_directories(hello PRIVATE
    "$ENV{ZVB_SDK_PATH}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_options(hello PRIVATE -m)
set_property(SOURCE src/hello.asm APPEND PROPERTY OBJECT_DEPENDS
    "$ENV{ZVB_SDK_PATH}/include/zvb_hardware_h.asm"
)
