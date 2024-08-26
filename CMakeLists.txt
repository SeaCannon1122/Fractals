cmake_minimum_required(VERSION 3.15)
project(Fractals)

set(SOURCE_FILES
    src/platform.c
    src/utils.c
    src/fractal.c
    src/main.c
    
)

include_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/inc)
link_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/lib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Y-)
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-implicit-function-declaration)
endif()

set_target_properties(Fractals PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

target_link_libraries(Fractals OpenCL)

