cmake_minimum_required(VERSION 3.16)

add_library(cascades_hoge SHARED hoge.c)
target_include_directories(cascades_hoge PRIVATE ${CMAKE_SOURCE_DIR})
add_library(cascades_fuga SHARED fuga.c)
target_include_directories(cascades_fuga PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(cascades_fuga cascades_hoge)
add_executable(cascades_main main.c)
target_compile_options(cascades_main PUBLIC -fno-builtin)
target_link_libraries(cascades_main cascades_fuga -nostdlib)

add_test(
  NAME cascades_test 
  COMMAND ./sloader ./tests/cascades/cascades_main
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
