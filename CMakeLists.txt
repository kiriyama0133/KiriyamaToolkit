cmake_minimum_required(VERSION 3.20)
project(kiriyamaToolkit VERSION 0.1.0 DESCRIPTION
    "kiriyamaToolkit"
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    add_compile_options(/utf-8)
endif()
add_subdirectory(core)
add_executable(kiriyamaToolkit src/main.c)
target_link_libraries(kiriyamaToolkit kiriyamaToolkit::core)
