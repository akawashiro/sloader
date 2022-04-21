cmake_minimum_required(VERSION 3.16)

add_executable(hello_ET_DYN main.c)

add_test(
  NAME hello_ET_DYN_test
  COMMAND ./sloader ./tests/hello_ET_DYN/hello_ET_DYN
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
