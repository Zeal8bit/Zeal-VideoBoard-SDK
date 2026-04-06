cmake_minimum_required(VERSION 3.16)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Input directory for the source files
set(INPUT_DIR ${CMAKE_SOURCE_DIR}/sdcc)
set(GFX_DIR ${INPUT_DIR}/gfx)


set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

project(zvb-sdk-libs C)

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
zvb_add_library(zvb_gfx
    ${GFX_DIR}/core.c
    ${GFX_DIR}/palette.c
    ${GFX_DIR}/tilemap/load.c
    ${GFX_DIR}/tilemap/place.c
    ${GFX_DIR}/tileset/load.c
    ${GFX_DIR}/tileset/add_color_tile.c
    ${GFX_DIR}/sprite/render.c
    ${GFX_DIR}/sprite/render_array.c
    ${GFX_DIR}/sprite/set_xy.c
    ${GFX_DIR}/sprite/set_x.c
    ${GFX_DIR}/sprite/set_y.c
    ${GFX_DIR}/sprite/set_tile.c
    ${GFX_DIR}/sprite/set_flags.c
)
zvb_add_library(zvb_crc   ${INPUT_DIR}/zvb_crc.c)
zvb_add_library(zvb_sound ${INPUT_DIR}/zvb_sound.c)
zvb_add_library(zvb_dma   ${INPUT_DIR}/zvb_dma.c)

zvb_add_library(zvb
    ${GFX_DIR}/core.c
    ${GFX_DIR}/palette.c
    ${GFX_DIR}/tilemap/load.c
    ${GFX_DIR}/tilemap/place.c
    ${GFX_DIR}/tileset/load.c
    ${GFX_DIR}/tileset/add_color_tile.c
    ${GFX_DIR}/sprite/render.c
    ${GFX_DIR}/sprite/render_array.c
    ${GFX_DIR}/sprite/set_xy.c
    ${GFX_DIR}/sprite/set_x.c
    ${GFX_DIR}/sprite/set_y.c
    ${GFX_DIR}/sprite/set_tile.c
    ${GFX_DIR}/sprite/set_flags.c
    ${INPUT_DIR}/zvb_sound.c
    ${INPUT_DIR}/zvb_crc.c
    ${INPUT_DIR}/zvb_dma.c
)

zvb_add_symbol_report(zvb)

# Group target to build all
add_custom_target(all_libs DEPENDS zvb zvb_gfx zvb_crc zvb_sound zvb_dma)
