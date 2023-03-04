cmake_minimum_required(VERSION 3.21)
project(path_tracing)

file(GLOB SRC_FILES
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glm/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glad/*.c
        ${PROJECT_SOURCE_DIR}/src/*.h
        ${PROJECT_SOURCE_DIR}/src/*.c
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        )

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
link_directories(lib)
add_executable(path_tracing ${SRC_FILES} )
target_link_libraries(path_tracing opengl32 glu32 libglfw3.a libzlibstatic.a)
