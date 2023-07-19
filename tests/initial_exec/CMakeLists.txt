cmake_minimum_required(VERSION 3.16)

add_library(initial_exec_hoge SHARED hoge.c)
add_executable(initial_exec main.c)
target_link_libraries(initial_exec initial_exec_hoge)

add_test(
  NAME initial_exec
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/initial_exec/initial_exec)
set_tests_properties(initial_exec PROPERTIES WILL_FAIL TRUE)
