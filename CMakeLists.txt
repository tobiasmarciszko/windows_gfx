cmake_minimum_required(VERSION 3.5)

project(win_gfx CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wno-c++98-compat -Wno-c++98-compat-pedantic)
add_definitions(-DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
add_executable(win_gfx WIN32 main.cpp)
