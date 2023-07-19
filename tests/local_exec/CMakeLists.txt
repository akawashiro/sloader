cmake_minimum_required(VERSION 3.16)

add_executable(local_exec main.c)

add_test(
  NAME local_exec
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/local_exec/local_exec)
set_tests_properties(local_exec PROPERTIES WILL_FAIL TRUE)
