if (NOT DEFINED ZVB_SDK_DIR)
    set(ZVB_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/tiled2zeal.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/gif2zeal.cmake)