cmake_minimum_required(VERSION 2.6)
project(thunderborg)

add_library(thunderborg SHARED
    src/ThunderBorg.cpp
)

add_executable(test_motors test_motors.cpp)
add_executable(update_board_addr update_board_addr.cpp)

target_link_libraries(test_motors thunderborg)
target_link_libraries(update_board_addr thunderborg)