cmake_minimum_required(VERSION 3.16)

add_library(dlopen_hoge SHARED hoge.c)
add_executable(dlopen_main main.c)

add_test(
  NAME dlopen_test
  COMMAND
    python3 ${CMAKE_SOURCE_DIR}/tests/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader ${CMAKE_BINARY_DIR}/tests/dlopen/dlopen_main
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/dlopen)
