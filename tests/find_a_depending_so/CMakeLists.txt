cmake_minimum_required(VERSION 3.16)

add_library(hoge SHARED hoge.c)
target_include_directories(hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(find_a_depending_so main.c)
target_compile_options(find_a_depending_so PUBLIC -fno-builtin)
target_link_libraries(find_a_depending_so hoge -nostdlib)

add_test(
  NAME find_a_depending_so_test 
  COMMAND ./sloader ./tests/find_a_depending_so/find_a_depending_so
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
