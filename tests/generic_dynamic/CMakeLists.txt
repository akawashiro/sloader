cmake_minimum_required(VERSION 3.16)

add_library(generic_dyanmic_hoge SHARED hoge.c)
add_executable(generic_dyanmic main.c)
target_link_libraries(generic_dyanmic generic_dyanmic_hoge)

add_test(
  NAME generic_dyanmic
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/generic_dyanmic/generic_dyanmic)

set_tests_properties(
  generic_dyanmic
  PROPERTIES WILL_FAIL TRUE)
