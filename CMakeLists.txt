cmake_minimum_required(VERSION 3.16)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Input directory for the source files
set(INPUT_DIR ${CMAKE_SOURCE_DIR}/sdcc)
set(GFX_DIR ${INPUT_DIR}/gfx)
set(SOUND_DIR ${INPUT_DIR}/sound)
set(CRC_DIR ${INPUT_DIR})
set(DMA_DIR ${INPUT_DIR})

file(GLOB GFX_SOURCES CONFIGURE_DEPENDS
    ${GFX_DIR}/*.c
    ${GFX_DIR}/tilemap/*.c
    ${GFX_DIR}/tileset/*.c
    ${GFX_DIR}/sprite/*.c
)
list(SORT GFX_SOURCES)

file(GLOB SOUND_SOURCES CONFIGURE_DEPENDS
    ${SOUND_DIR}/*.c
)
list(SORT SOUND_SOURCES)

set(CRC_SOURCES
    ${CRC_DIR}/zvb_crc.asm
    # ${CRC_DIR}/zvb_crc.c
)

set(DMA_SOURCES
    ${DMA_DIR}/zvb_dma.asm
    # ${DMA_DIR}/zvb_dma.c
)


set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

project(zvb-sdk-libs C ASM)

# Function to create libraries
function(zvb_add_library target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

function(zvb_add_symbol_report target_name)
    set(report_dir ${CMAKE_SOURCE_DIR}/debug)
    set(report_file ${report_dir}/${target_name}.sym)

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${report_dir}
        COMMAND /bin/sh -c "sdnm -sS --size-sort '$<TARGET_FILE:${target_name}>' > '${report_file}'"
        COMMENT "Generating symbol size report: ${report_file}"
        VERBATIM
    )
endfunction()

# Create each ZVB library
zvb_add_library(zvb_gfx ${GFX_SOURCES})
zvb_add_library(zvb_crc   ${CRC_SOURCES})
zvb_add_library(zvb_sound ${SOUND_SOURCES})
zvb_add_library(zvb_dma   ${DMA_SOURCES})

zvb_add_library(zvb
    ${GFX_SOURCES}
    ${SOUND_SOURCES}
    ${CRC_SOURCES}
    ${DMA_SOURCES}
)

zvb_add_symbol_report(zvb)

# Group target to build all
add_custom_target(all_libs DEPENDS zvb zvb_gfx zvb_crc zvb_sound zvb_dma)
