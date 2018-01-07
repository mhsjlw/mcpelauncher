cmake_minimum_required(VERSION 2.8)
project(hybris)

enable_language(C CXX ASM)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -m32")
set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")

add_subdirectory(epoll)
add_subdirectory(glad)

include_directories(rapidxml/ hybris/include epoll/include glad/include)
set(HYBRIS_SOURCES hybris/src/cache.c hybris/src/dlfcn.c hybris/src/hooks.c hybris/src/hooks.cpp hybris/src/hooks_shm.c hybris/src/logging.c hybris/src/properties.c hybris/src/ctype.c hybris/src/sysconf.c hybris/src/jb/dlfcn.c hybris/src/jb/linker.c hybris/src/jb/linker_environ.c hybris/src/jb/linker_format.c hybris/src/jb/rt.c)
add_library(hybris STATIC ${HYBRIS_SOURCES})
add_dependencies(hybris epoll)
target_compile_definitions(hybris PRIVATE ANDROID_X86_LINKER _GNU_SOURCE LINKER_TEXT_BASE=0xB0000100 LINKER_AREA_SIZE=0x01000000 LINKER_DEBUG=3)
