set(CMAKE_C_STANDARD 11)

include_directories(../../c/src)

file(GLOB test_src_files *.c)
foreach(file ${test_src_files})
    set(file_name ${file})
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${file})
    project(${file_name}_C "C")
    add_executable(${file_name}_C ${file})
    target_link_libraries(${file_name}_C ${PGBN_C_STATIC_LIB} ${PGBN_STATIC_LIB} ${STD_CXX_LIB})
endforeach(file)
