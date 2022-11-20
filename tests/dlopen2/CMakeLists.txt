cmake_minimum_required(VERSION 3.16)

add_library(dlopen2_hoge SHARED hoge.c)
add_executable(dlopen2_main main.c)
target_link_options(dlopen2_main PRIVATE -export-dynamic)

add_test(
  NAME dlopen2_test
  COMMAND
    python3 ${CMAKE_SOURCE_DIR}/tests/compare_stdout.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_CURRENT_BINARY_DIR}/dlopen2_main
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
