cmake_minimum_required(VERSION 3.23)
project(webserver C)

set(CMAKE_C_STANDARD 11)

add_executable(webserver
        "src/main.c"
        "src/event_loop.c"
        "src/connection.c"
)

add_library(webserver_headers INTERFACE)
target_sources(webserver_headers INTERFACE
        "include/app/error.h"
        "include/app/event_loop.h"
        "include/app/connection.h"
        "include/app/channel.h"
)

target_link_libraries(webserver PRIVATE webserver_headers)
target_include_directories(webserver PRIVATE "include")
target_compile_definitions(webserver PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(webserver PRIVATE "-fno-strict-aliasing")
endif()
