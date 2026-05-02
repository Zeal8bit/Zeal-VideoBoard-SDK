#
# tiled2zeal(
#   <target>
#   [COMPRESSED]
#   [VERBOSE]
#   [DEBUG]
#   [LAYER <layer-name>]
#   [SIZE <tile-size>]
#   [OUTPUT <output-path>]
#   FILES <tmx> [<tmx>...]
# )
# <target>: Existing CMake target that will depend on the generated assets.
# COMPRESSED: Enable compression for the generated tilemap output.
# VERBOSE: Enable verbose output from the conversion script.
# DEBUG: Enable debug output from the conversion script.
# LAYER <layer-name>: Export the named TMX layer.
# SIZE <tile-size>: Set the tile size passed to the conversion script.
# OUTPUT <output-path>: Write the generated `.ztm` to a specific path.
# FILES <tmx> [<tmx>...]: One or more input TMX files to convert.
function(tiled2zeal target)
    set(target_name ${ARGV0})
    # Remove target name for parsing
    list(REMOVE_AT ARGV 0)

    # Parse arguments
    set(_FLAGS COMPRESSED VERBOSE DEBUG)
    set(_KEYS LAYER SIZE OUTPUT)

    cmake_parse_arguments(
        TILED2ZEAL
        "${_FLAGS}"
        "${_KEYS}"
        FILES
        ${ARGV}
    )

    set(extra_args_list "")  # reserved for future flags/keys
    if(TILED2ZEAL_COMPRESSED)
        list(APPEND extra_args_list "-z")
    endif()
    if(TILED2ZEAL_VERBOSE)
        list(APPEND extra_args_list "-v")
    endif()
    if(TILED2ZEAL_DEBUG)
        list(APPEND extra_args_list "-d")
    endif()
    if(TILED2ZEAL_LAYER) # Bits Per Pixel
        list(APPEND extra_args_list "-l" "${TILED2ZEAL_LAYER}")
    endif()
    if(TILED2ZEAL_SIZE) # Bits Per Pixel
        list(APPEND extra_args_list "-s" "${TILED2ZEAL_SIZE}")
    endif()


    set(GENERATED_ASSETS)
    if(TILED2ZEAL_OUTPUT)
        list(LENGTH TILED2ZEAL_FILES tiled2zeal_file_count)
        if(tiled2zeal_file_count GREATER 1)
            message(FATAL_ERROR "tiled2zeal OUTPUT can only be used with one TMX file")
        endif()
    endif()

    foreach(tmx ${TILED2ZEAL_FILES})
        get_filename_component(fname_we ${tmx} NAME_WE)
        get_filename_component(tmx_abs ${tmx} ABSOLUTE)
        string(REPLACE "." "_" fname_safe ${fname_we})

        if(TILED2ZEAL_OUTPUT)
            if(IS_ABSOLUTE "${TILED2ZEAL_OUTPUT}")
                set(output_path "${TILED2ZEAL_OUTPUT}")
            else()
                set(output_path "${CMAKE_BINARY_DIR}/${TILED2ZEAL_OUTPUT}")
            endif()
        else()
            set(output_path "${CMAKE_BINARY_DIR}/assets/${fname_we}.ztm")
        endif()

        get_filename_component(build_out_dir "${output_path}" DIRECTORY)
        set(stamp ${CMAKE_BINARY_DIR}/CMakeFiles/${target}_${fname_safe}_tiled_asset.stamp)

        # Make a unique target name based on TMX filename
        set(custom_target_name "${target}_${fname_safe}_tiled_asset")

        add_custom_command(
            OUTPUT ${stamp} ${output_path}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${build_out_dir}
            COMMAND ${Python3_EXECUTABLE} $ENV{ZVB_SDK_PATH}/tools/tiled2zeal/tiled2zeal.py
                    -i ${tmx_abs}
                    -o ${output_path}
                    ${extra_args_list}
            COMMAND ${CMAKE_COMMAND} -E touch ${stamp}
            DEPENDS ${tmx_abs} $ENV{ZVB_SDK_PATH}/tools/tiled2zeal/tiled2zeal.py
            COMMENT "Processing TMX asset ${fname_we}.tmx"
            VERBATIM
        )

        list(APPEND GENERATED_ASSETS ${stamp})

        # Per-file custom target
        add_custom_target(${custom_target_name} ALL DEPENDS ${stamp})
        add_dependencies(${target_name} ${custom_target_name})
    endforeach()
endfunction()
