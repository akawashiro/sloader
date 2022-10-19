cmake_minimum_required(VERSION 3.16)

add_test(
  NAME coreutils_cat
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader)
