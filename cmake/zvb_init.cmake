if (NOT DEFINED ZVB_SDK_DIR)
    set(ZVB_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()


function(gif2zeal target)
    set(target_name ${ARGV0})
    # Remove the target name from arguments for parsing
    list(REMOVE_AT ARGV 0)

    # Parse arguments
    set(_FLAGS COMPRESSED UNIQUE TILEMAP VERBOSE DEBUG)
    set(_KEYS BIT COLORS STRIP)

    cmake_parse_arguments(
        GIF2ZEAL
        "${_FLAGS}"
        "${_KEYS}"
        FILES
        ${ARGV}
    )

    # Compose extra_args_list for python script
    set(extra_args_list "")
    if(GIF2ZEAL_COMPRESSED) # Compress with RLE
        list(APPEND extra_args_list "-z")
    endif()
    if(GIF2ZEAL_UNIQUE) # Remove duplicate tiles
        list(APPEND extra_args_list "-u")
    endif()
    if(GIF2ZEAL_VERBOSE) # Verbose output
        list(APPEND extra_args_list "-v")
    endif()
    if(GIF2ZEAL_DEBUG) # Debug Output
        list(APPEND extra_args_list "-d")
    endif()
    if(GIF2ZEAL_BIT) # Bits Per Pixel
        list(APPEND extra_args_list "-b" "${GIF2ZEAL_BIT}")
    endif()
    if(GIF2ZEAL_COLORS) # Max colors in Palette
        list(APPEND extra_args_list "-c" "${GIF2ZEAL_COLORS}")
    endif()
    if(GIF2ZEAL_STRIP) # Strip N tiles off the end
        list(APPEND extra_args_list "-s" "${GIF2ZEAL_STRIP}")
    endif()


    set(gif_files ${GIF2ZEAL_FILES})

    set(GENERATED_ASSETS)
    foreach(gif ${gif_files})
        get_filename_component(fname_we ${gif} NAME_WE)
        get_filename_component(gif_abs ${gif} ABSOLUTE)

        # Output files relative to build dir for SDCC incbin
        set(build_out_dir ${CMAKE_BINARY_DIR}/assets)
        set(zts ${build_out_dir}/${fname_we}.zts)
        set(ztp ${build_out_dir}/${fname_we}.ztp)
        if(GIF2ZEAL_TILEMAP)  # Only define ZTM if requested
            set(ztm ${build_out_dir}/${fname_we}.ztm)
        else()
            unset(ztm)
        endif()

        # Make a unique target name based on filename
        string(REPLACE "." "_" fname_safe ${fname_we})
        set(custom_target_name "${target}_${fname_safe}_asset")

        # Build the Python command as a proper list
        set(cmd ${Python3_EXECUTABLE} $ENV{ZVB_SDK_PATH}/tools/zeal2gif/gif2zeal.py)
        list(APPEND cmd -i ${build_out_dir}/${fname_we}.gif)
        list(APPEND cmd -t ${zts} -p ${ztp})
        if(DEFINED ztm)
            list(APPEND cmd -m ${ztm})
        endif()
        list(APPEND cmd -o ${build_out_dir})
        list(APPEND cmd ${extra_args_list})

        add_custom_command(
            OUTPUT ${zts} ${ztp} $<$<BOOL:${ztm}>:${ztm}>
            COMMAND ${CMAKE_COMMAND} -E make_directory ${build_out_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gif_abs} ${build_out_dir}/${fname_we}.gif
            COMMAND ${cmd}
            DEPENDS ${gif_abs} $ENV{ZVB_SDK_PATH}/tools/zeal2gif/gif2zeal.py
            COMMENT "Processing asset ${fname_we}.gif"
            VERBATIM
        )

        set(target_deps ${zts} ${ztp})
        if(DEFINED ztm)
            list(APPEND target_deps ${ztm})
        endif()
        list(APPEND GENERATED_ASSETS ${target_deps})

        # Create a per-file custom target
        add_custom_target(${custom_target_name} ALL DEPENDS ${zts} ${ztp} ${ztm})
        add_dependencies(${target_name} ${custom_target_name})
    endforeach()
endfunction()


function(tiled2zeal target)
    set(target_name ${ARGV0})
    # Remove target name for parsing
    list(REMOVE_AT ARGV 0)

    # Parse arguments
    set(_FLAGS COMPRESSED VERBOSE DEBUG)
    set(_KEYS LAYER SIZE)

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
    foreach(tmx ${TILED2ZEAL_FILES})
        get_filename_component(fname_we ${tmx} NAME_WE)
        get_filename_component(tmx_abs ${tmx} ABSOLUTE)

        # Output file relative to build directory for SDCC incbin
        set(build_out_dir ${CMAKE_BINARY_DIR}/assets)
        set(ztm ${build_out_dir}/${fname_we}.ztm)

        # Make a unique target name based on TMX filename
        string(REPLACE "." "_" fname_safe ${fname_we})
        set(custom_target_name "${target}_${fname_safe}_tiled_asset")

        add_custom_command(
            OUTPUT ${ztm}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${build_out_dir}
            COMMAND ${Python3_EXECUTABLE} $ENV{ZVB_SDK_PATH}/tools/tiled2zeal/tiled2zeal.py
                    -i ${tmx_abs}
                    -o ${build_out_dir}
                    ${extra_args_list}
            DEPENDS ${tmx_abs} $ENV{ZVB_SDK_PATH}/tools/tiled2zeal/tiled2zeal.py
            COMMENT "Processing TMX asset ${fname_we}.tmx"
            VERBATIM
        )

        list(APPEND GENERATED_ASSETS ${ztm})

        # Per-file custom target
        add_custom_target(${custom_target_name} ALL DEPENDS ${ztm})
        add_dependencies(${target_name} ${custom_target_name})
    endforeach()
endfunction()
