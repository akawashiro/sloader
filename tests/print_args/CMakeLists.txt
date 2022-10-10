cmake_minimum_required(VERSION 3.16)

add_executable(print_args main.c)

add_test(
  NAME print_args
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/print_args/print_args hoge fuga)
