cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(PRJ_NAME v4l2_udp_server)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
project(PRJ_NAME C CXX ASM)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -g3
        -O0
        )


add_subdirectory(test)