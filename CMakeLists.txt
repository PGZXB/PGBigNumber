set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

set(LIBS_FOR_TEST PGBigNumber)

add_definitions(-DPGBN_DEBUG)
add_definitions(-DPGZXB_DEBUG)
add_definitions(-Wall)
add_subdirectory(test)
add_subdirectory(c)
include_directories(include)

file(GLOB_RECURSE  src_files src/*.cpp)

# find_file(MAIN_FILE_PATH main.cpp ./src/)
# message("main.cpp path : " ${MAIN_FILE_PATH})
# list(REMOVE_ITEM src_files ${MAIN_FILE_PATH})

project(PGBigNumber)
add_library(PGBigNumber STATIC ${src_files})