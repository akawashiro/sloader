cmake_minimum_required(VERSION 3.16)

add_library(constructor_hoge SHARED hoge.c)
target_include_directories(constructor_hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(constructor_main main.c)
target_compile_options(constructor_main PUBLIC -fno-builtin)
target_link_libraries(constructor_main constructor_hoge -nostdlib)

add_test(
  NAME constructor_test 
  COMMAND ./sloader ./tests/constructor/constructor_main
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
