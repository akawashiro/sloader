cmake_minimum_required(VERSION 3.16)

add_library(global_variable_hoge SHARED hoge.c)
target_include_directories(global_variable_hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(global_variable_main main.c)
target_compile_options(global_variable_main PUBLIC -fno-builtin)
target_link_libraries(global_variable_main global_variable_hoge -nostdlib)

add_test(
  NAME global_variable_test
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/global_variable/global_variable_main)
