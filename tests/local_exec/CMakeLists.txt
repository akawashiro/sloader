cmake_minimum_required(VERSION 3.16)

add_executable(local_exec main.c)

add_test(
  NAME tls
  COMMAND ./sloader ./tests/local_exec/local_exec
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
