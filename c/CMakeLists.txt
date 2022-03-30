set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_STANDARD 11)

add_definitions(-DPGBN_DEBUG)
add_definitions(-DPGZXB_DEBUG)
add_definitions(-Wall)

project(${PGBN_C_STATIC_LIB})
add_library(${PGBN_C_STATIC_LIB} STATIC src/pgbn.cpp)
target_link_libraries(${PGBN_C_STATIC_LIB} PUBLIC ${PGBN_STATIC_LIB})
