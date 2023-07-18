cmake_minimum_required(VERSION 3.16)

add_executable(errno main.c)

add_test(
  NAME errno
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py
          ${CMAKE_BINARY_DIR}/sloader ${CMAKE_BINARY_DIR}/tests/errno/errno)
