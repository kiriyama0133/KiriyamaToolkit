if(WIN32)
    set(PLATFORM_SOURCE
        src/platform/windows/platform_windows.c
    )
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_SOURCE
        src/platform/linux/platform_linux.c
    )
endif()

add_library(kiriyamaToolkit_core STATIC
    src/runtime.c
    ${PLATFORM_SOURCE}
)
add_library(kiriyamaToolkit::core ALIAS kiriyamaToolkit_core)
target_include_directories(kiriyamaToolkit_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)