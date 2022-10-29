cmake_minimum_required(VERSION 3.16)

add_library(local_dynamic_hoge SHARED hoge.c)
add_executable(local_dynamic main.c)
target_link_libraries(local_dynamic local_dynamic_hoge)

add_test(
  NAME local_dynamic
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/local_dynamic/local_dynamic)

set_tests_properties(
  local_dynamic
  PROPERTIES WILL_FAIL TRUE)
