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

if(WIN32)
    set_target_properties(Fractals PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(Fractals X11 pthread m)
endif()



target_link_libraries(Fractals OpenCL)

