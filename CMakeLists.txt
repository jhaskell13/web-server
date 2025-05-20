cmake_minimum_required(VERSION 3.10)

project(HttpServer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
    server.c
    http.c
    mime.c
    threadpool.c
)

add_executable(serve ${SOURCES})
