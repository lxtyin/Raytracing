﻿cmake_minimum_required(VERSION 3.21)
project(path_tracing)

file(GLOB SRC_FILES
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/include/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/backend/*.cpp
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glm/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.h
        ${PROJECT_SOURCE_DIR}/src/tool/*.h
        ${PROJECT_SOURCE_DIR}/src/tool/*.hpp
        ${PROJECT_SOURCE_DIR}/src/tool/*.cpp
        ${PROJECT_SOURCE_DIR}/src/instance/*.h
        ${PROJECT_SOURCE_DIR}/src/instance/*.cpp
        ${PROJECT_SOURCE_DIR}/src/material/*.h
        ${PROJECT_SOURCE_DIR}/src/material/*.cpp
        ${PROJECT_SOURCE_DIR}/src/texture/*.h
        ${PROJECT_SOURCE_DIR}/src/texture/*.cpp
        ${PROJECT_SOURCE_DIR}/src/renderpass/*.h
        ${PROJECT_SOURCE_DIR}/src/renderpass/*.cpp
        ${PROJECT_SOURCE_DIR}/src/glad/*.c
        )

set(CMAKE_CXX_STANDARD 17)
set(OpenCV_DIR D:/OpenCV)
FIND_PACKAGE(OpenCV REQUIRED)


include_directories(include ${OpenCV_INCLUDE_DIRS})
link_directories(lib)
add_executable(path_tracing ${SRC_FILES}
        src/renderpass/StaticBlender.h
        src/renderpass/SVGFMergePass.h
        src/renderpass/SVGFVarianceFilter.h
)

#target_link_libraries(path_tracing opengl32 glu32 libglfw3.a libzlibstatic.a libassimp.dll.a ${OpenCV_LIBS})
target_link_libraries(path_tracing opengl32 glu32 assimp-vc143-mt.lib ${OpenCV_LIBS} glfw3.dll)

