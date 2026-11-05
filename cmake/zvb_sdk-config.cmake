# Called by: find_package(ZVB_SDK REQUIRED)

set(libraries zvb zvb_crc zvb_dma zvb_gfx zvb_sound)

foreach(lib ${libraries})
    if(TARGET ${lib})
        continue()
    endif()

    add_library(${lib} INTERFACE IMPORTED)

    set_target_properties(${lib} PROPERTIES
        IMPORTED_LIBNAME "${lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../include"
        INTERFACE_LINK_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../lib"
    )
endforeach()
