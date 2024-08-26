cmake_minimum_required(VERSION 3.15)
project(Fractals)

set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)


set(SOURCE_FILES
    src/platform.c
    src/openclutils.c
    src/utils.c
    src/fractal.c
    src/main.c
    
)

include_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/inc)
link_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/lib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set_target_properties(Fractals PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

target_link_libraries(Fractals OpenCL)

