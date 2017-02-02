cmake_minimum_required(VERSION 2.8)
project(eventus_test CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "(Clang)|(GNU)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    add_compile_options("-Wfatal-errors")
endif()

add_executable(test
    basic.cpp
    keys.cpp
    arguments.cpp
    removal.cpp
    lazytype.cpp
    other.cpp
)
