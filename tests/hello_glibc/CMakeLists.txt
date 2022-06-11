cmake_minimum_required(VERSION 3.16)

add_executable(hello_glibc main.c)

add_test(
  NAME hello_glibc
  COMMAND ./sloader ./tests/hello_glibc/hello_glibc
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
