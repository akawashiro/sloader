cmake_minimum_required(VERSION 3.16)
project(sloader)

enable_testing()

add_executable(sloader sloader.cc)
target_link_libraries(sloader glog)

add_executable(hello_libc hello.c)
target_link_libraries(hello_libc -static)
add_test(
  NAME hello_libc_test
  COMMAND ./sloader --load hello_libc
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(hello_libc_test PROPERTIES ENVIRONMENT "GLOG_logtostderr=1")

add_custom_target(
  hello_asm ALL
  COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/hello.asm -o
          ${CMAKE_CURRENT_BINARY_DIR}/hello.o && ld -s -o hello_asm hello.o
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(
  NAME hello_asm_test
  COMMAND ./sloader --load hello_asm
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(hello_asm_test PROPERTIES ENVIRONMENT "GLOG_logtostderr=1")
