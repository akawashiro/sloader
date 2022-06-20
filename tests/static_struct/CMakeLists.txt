cmake_minimum_required(VERSION 3.16)

add_library(static_struct_hoge SHARED hoge.c)
target_include_directories(static_struct_hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(static_struct_main main.c)
target_compile_options(static_struct_main PUBLIC -fno-builtin)
target_link_libraries(static_struct_main static_struct_hoge -nostdlib)

add_test(
  NAME static_struct_test 
  COMMAND ./sloader ./tests/static_struct/static_struct_main
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
