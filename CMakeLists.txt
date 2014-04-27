project(rfs)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=gnu++0x -Wall -Wextra -Wwrite-strings -Werror -pedantic -Wno-long-long")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-braces")
# From the Debian hardening guide
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -Wformat-security")

add_subdirectory(proto)
include_directories(${RfsProto_INCLUDE_DIRS})

add_subdirectory(lib)
include_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
