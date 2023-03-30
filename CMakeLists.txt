cmake_minimum_required(VERSION 3.21)
project(path_tracing)

file(GLOB SRC_FILES
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/include/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/backend/*.cpp
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glm/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glad/*.c
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/instance/*.cpp
        ${PROJECT_SOURCE_DIR}/src/texture/*.cpp
        ${PROJECT_SOURCE_DIR}/src/tool/*.cpp
        )

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
link_directories(lib)
add_executable(path_tracing ${SRC_FILES} src/texture/SimpleTexture.cpp src/texture/SimpleTexture.h src/texture/CubeBox.cpp src/texture/CubeBox.h src/texture/HDRTexture.cpp src/texture/HDRTexture.h)
target_link_libraries(path_tracing opengl32 glu32 libglfw3.a libzlibstatic.a libassimp.dll.a)
