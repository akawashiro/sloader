cmake_minimum_required(VERSION 3.16)

add_library(hoge SHARED hoge.c)
add_executable(find_a_depending_so main.c)
target_link_libraries(find_a_depending_so hoge)

add_test(
  NAME find_a_depending_so_test 
  COMMAND ./sloader ./tests/find_a_depending_so/find_a_depending_so
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
