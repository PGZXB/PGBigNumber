set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_STANDARD 11)

add_definitions(-DPGBN_DEBUG)
add_definitions(-DPGZXB_DEBUG)
add_definitions(-Wall)

project(PGBigNumberBindingC)
add_library(PGBigNumberBindingC STATIC src/pgbn.cpp)
target_link_libraries(PGBigNumberBindingC PUBLIC PGBigNumber)