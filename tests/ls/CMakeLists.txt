cmake_minimum_required(VERSION 3.16)

add_executable(ls ls.c)

add_test(
  NAME ls_test
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
          ${CMAKE_BINARY_DIR}/tests/ls/ls
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/ls)
