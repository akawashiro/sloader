cmake_minimum_required(VERSION 3.16)

add_library(generic_dynamic_hoge SHARED hoge.c)
add_executable(generic_dynamic main.c)
target_link_libraries(generic_dynamic generic_dynamic_hoge)

add_test(
  NAME generic_dynamic
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/generic_dynamic/generic_dynamic)
