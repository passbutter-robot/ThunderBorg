cmake_minimum_required(VERSION 2.6)
project(thunderborg)

add_library(thunderborg SHARED
    src/ThunderBorg.cpp
)

add_executable(test main.cpp)

target_link_libraries(test thunderborg)
