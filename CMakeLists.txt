cmake_minimum_required(VERSION 3.25)
project(Space-Travel)

set(CMAKE_CXX_STANDARD 20)
set(SDL2_INCLUDE_DIR C:/Users/DIEAL/OneDrive/Documents/SDL2-2.28.1/include)
set(SDL2_LIB_DIR C:/Users/DIEAL/OneDrive/Documents/SDL2-2.28.1/lib/x64)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

add_executable(Space-Travel main.cpp color.h print.h triangle.h uniform.h shaders.h fragment.h FastNoise.h FastNoiseLite.h ObjLoader.cpp camera.h framebuffer.h line.h noise.h model.h)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2)