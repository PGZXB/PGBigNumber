set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

set(PGBN_PROJECT_NAME PGBigNumber)
set(LIBS_FOR_TEST ${PGBN_PROJECT_NAME})
set(PGBN_STATIC_LIB ${PGBN_PROJECT_NAME})
set(PGBN_C_STATIC_LIB PGBigNumberBindingC)
set(STD_CXX_LIB )

if(NOT WIN32)
    list(APPEND STD_CXX_LIB stdc++)
endif(NOT WIN32)

if($ENV{PGBN_DEBUG})
    if($ENV{PGBN_DEBUG} EQUAL "1")
        message("Debug mode")
        add_definitions(-DPGBN_DEBUG)
        add_definitions(-DPGZXB_DEBUG)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Open debug mode")
    else($ENV{PGBN_DEBUG} EQUAL "1")
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Release")
    endif($ENV{PGBN_DEBUG} EQUAL "1")
endif($ENV{PGBN_DEBUG})

if($ENV{PGBN_BUILD_TESTS})
    if($ENV{PGBN_BUILD_TESTS} EQUAL "1")
        add_subdirectory(tests)
    endif($ENV{PGBN_BUILD_TESTS} EQUAL "1")
endif($ENV{PGBN_BUILD_TESTS})

if($ENV{PGBN_BUILD_EXAMPLES})
    if($ENV{PGBN_BUILD_EXAMPLES} EQUAL "1")
        add_subdirectory(examples)
    endif($ENV{PGBN_BUILD_EXAMPLES} EQUAL "1")
endif($ENV{PGBN_BUILD_EXAMPLES})

add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-Wall)
add_subdirectory(c)

file(GLOB_RECURSE  pgbn_src_files src/*.cpp)
project(${PGBN_PROJECT_NAME})
add_library(${PGBN_PROJECT_NAME} STATIC ${pgbn_src_files})

find_package(Threads)
target_link_libraries(${PGBN_PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
