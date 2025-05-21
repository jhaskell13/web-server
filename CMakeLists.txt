cmake_minimum_required(VERSION 3.10)

project(HttpServer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES "src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(serve ${SOURCES})
