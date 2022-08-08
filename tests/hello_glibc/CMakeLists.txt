cmake_minimum_required(VERSION 3.16)

add_executable(hello_glibc main.c)

add_test(
  NAME hello_glibc
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/hello_glibc/hello_glibc)
