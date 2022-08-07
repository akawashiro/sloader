cmake_minimum_required(VERSION 3.16)

add_library(constructor_hoge SHARED hoge.c)
target_include_directories(constructor_hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(constructor_main main.c)
target_compile_options(constructor_main PUBLIC -fno-builtin)
target_link_libraries(constructor_main constructor_hoge -nostdlib)

add_test(
  NAME constructor_test
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/constructor/constructor_main)
