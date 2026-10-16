cmake_minimum_required(VERSION 3.23)
project(webserver C)

set(CMAKE_C_STANDARD 11)

add_executable(webserver
        main.c
        event_loop.c
        source.h
        error.h
        event_loop.h
        listen_socket.c
        listen_socket.h
        source.c)

target_compile_definitions(webserver PRIVATE $<$<CONFIG:Debug>:DEBUG>)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(webserver PRIVATE "-fno-strict-aliasing")
endif()
