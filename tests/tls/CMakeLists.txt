cmake_minimum_required(VERSION 3.16)

add_executable(tls main.c)

add_test(
  NAME tls
  COMMAND ./sloader ./tests/tls/tls
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
