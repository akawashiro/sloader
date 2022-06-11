cmake_minimum_required(VERSION 3.16)
project(sloader)

enable_testing()
set(CMAKE_CXX_STANDARD 20)
set(CUSTOM_LIBC_PATH CACHE PATH "")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(sloader sloader.cc dyn_loader.cc utils.cc)
target_link_options(sloader PUBLIC -static -T ${CMAKE_CURRENT_SOURCE_DIR}/sloader.map)
target_compile_options(sloader PUBLIC -Wall -Werror)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake
               ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)

if(CUSTOM_LIBC_PATH)
  add_custom_target(
    hello_static_custom_libc ALL
    COMMAND
      gcc -g -c ${CMAKE_CURRENT_SOURCE_DIR}/hello.c -o
      ${CMAKE_CURRENT_BINARY_DIR}/hello_static_custom_libc.o && gcc -o
      hello_static_custom_libc -nostdlib -nostartfiles -static
      ${CUSTOM_LIBC_PATH}/lib/crt1.o ${CUSTOM_LIBC_PATH}/lib/crti.o `gcc
      --print-file-name=crtbegin.o`
      ${CMAKE_CURRENT_BINARY_DIR}/hello_static_custom_libc.o -Wl,--start-group
      ${CUSTOM_LIBC_PATH}/lib/libc.a -lgcc -lgcc_eh -Wl,--end-group `gcc
      --print-file-name=crtend.o` ${CUSTOM_LIBC_PATH}/lib/crtn.o)
  add_test(
    NAME hello_static_custom_libc_test
    COMMAND ./sloader ./hello_static_custom_libc
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
