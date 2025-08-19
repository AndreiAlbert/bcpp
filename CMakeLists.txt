cmake_minimum_required(VERSION 3.10)
project(bcpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_COMPILER "clang++")

add_executable(bcpp main.cpp)

target_include_directories(bcpp PRIVATE ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
target_sources(bcpp PRIVATE ${SOURCES})

target_compile_options(bcpp PRIVATE -Wall -Wextra -Werror -g)

