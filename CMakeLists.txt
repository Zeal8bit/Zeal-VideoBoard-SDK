cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

project(zvb-sdk-libs C)

# Function to create libraries
function(zvb_add_library name src)
    add_library(${name} STATIC ${src})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

# Input directory for the source files
set(INPUT_DIR ${CMAKE_SOURCE_DIR}/sdcc)

# Create each ZVB library
zvb_add_library(zvb_gfx   ${INPUT_DIR}/zvb_gfx.c)
zvb_add_library(zvb_crc   ${INPUT_DIR}/zvb_crc.c)
zvb_add_library(zvb_sound ${INPUT_DIR}/zvb_sound.c)
zvb_add_library(zvb_dma   ${INPUT_DIR}/zvb_dma.c)

# Group target to build all
add_custom_target(all_libs DEPENDS zvb_gfx zvb_crc zvb_sound zvb_dma)
