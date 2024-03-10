cmake_minimum_required(VERSION 3.21)
project(path_tracing)

file(GLOB SRC_FILES
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/include/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/backend/*.cpp
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        ${PROJECT_SOURCE_DIR}/include/glm/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/instance/*.cpp
        ${PROJECT_SOURCE_DIR}/src/texture/*.cpp
        ${PROJECT_SOURCE_DIR}/src/tool/*.cpp
        ${PROJECT_SOURCE_DIR}/src/glad/*.c
        )

set(CMAKE_CXX_STANDARD 17)
set(OpenCV_DIR D:/OpenCV)
FIND_PACKAGE(OpenCV REQUIRED)


include_directories(include ${OpenCV_INCLUDE_DIRS})
link_directories(lib)
add_executable(path_tracing ${SRC_FILES} src/tool/exglfw.h src/tool/json.hpp src/instance/Camera.cpp src/instance/Camera.h
        src/texture/Texture.cpp
        src/GPUResource.cpp
        src/texture/Skybox.cpp
        src/material/RoughConductor.h
        src/material/RoughConductor.cpp
        src/AABB.cpp)

#target_link_libraries(path_tracing opengl32 glu32 libglfw3.a libzlibstatic.a libassimp.dll.a ${OpenCV_LIBS})
target_link_libraries(path_tracing opengl32 glu32 assimp-vc143-mt.lib ${OpenCV_LIBS} glfw3.dll)

