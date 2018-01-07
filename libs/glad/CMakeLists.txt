cmake_minimum_required(VERSION 2.8)
project(glad C)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")

include_directories(include/)
set(GLAD_SOURCES src/glad.c)
add_library(glad STATIC ${GLAD_SOURCES})
