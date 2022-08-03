cmake_minimum_required(VERSION 3.16)

add_library(initial_exec_hoge SHARED hoge.c)
add_executable(initial_exec main.c)
target_link_libraries(initial_exec initial_exec_hoge)

add_test(
  NAME tls
  COMMAND ./sloader ./tests/initial_exec/initial_exec
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
