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
set(OpenCV_DIR D:/OpenCV)
FIND_PACKAGE(OpenCV REQUIRED)


include_directories(include)
link_directories(lib)
add_executable(path_tracing ${SRC_FILES} src/tool/exglfw.h src/tool/json.hpp NerfCreator.h src/instance/Camera.cpp src/instance/Camera.h)
target_link_libraries(path_tracing opengl32 glu32 libglfw3.a libzlibstatic.a libassimp.dll.a ${OpenCV_LIBS})
