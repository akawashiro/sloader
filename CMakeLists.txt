cmake_minimum_required(VERSION 3.16)
project(sloader)

enable_testing()
set(CMAKE_CXX_STANDARD 20)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(CUSTOM_LIBC_PATH CACHE PATH "")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(third-party)
set(WITH_GFLAGS
    OFF
    CACHE INTERNAL "" FORCE)
set(WITH_UNWIND OFF)
set(BUILD_SHARED_LIBS OFF)
add_definitions(-DC10_USE_GLOG=1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/raw_write)

add_library(tls_secure tls_secure.cc)

if(CUSTOM_LIBC_PATH)
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtbegin.o
    OUTPUT_VARIABLE CRT_BEGIN
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtend.o
    OUTPUT_VARIABLE CRT_END
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libstdc++.a
    OUTPUT_VARIABLE LIBSTDCXX
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  add_executable(sloader sloader.cc dyn_loader.cc libc_mapping.cc sloader_dl.cc
                         utils.cc sloader_static.map)
  target_link_libraries(
    sloader
    ${CUSTOM_LIBC_PATH}/lib/crt1.o
    ${CUSTOM_LIBC_PATH}/lib/crti.o
    ${CRT_BEGIN}
    ${LIBSTDCXX}
    ${CUSTOM_LIBC_PATH}/lib/libpthread.a
    -Wl,--start-group
    ${CUSTOM_LIBC_PATH}/lib/libc.a
    -lgcc
    -lgcc_eh
    -Wl,--end-group
    ${CRT_END}
    ${CUSTOM_LIBC_PATH}/lib/crtn.o
    tls_secure)
  target_compile_options(sloader PUBLIC -Wall -Werror)
  target_link_options(
    sloader
    BEFORE
    PUBLIC
    -nostdlib
    -nostartfiles
    -static
    -Wl,-verbose
    -T
    ${CMAKE_CURRENT_SOURCE_DIR}/sloader_static.map)
else()
  add_executable(sloader sloader.cc dyn_loader.cc libc_mapping.cc sloader_dl.cc
                         utils.cc sloader_static.map)
  target_link_libraries(sloader tls_secure)
  target_compile_options(sloader PUBLIC -Wall -Werror)
  target_link_options(sloader PUBLIC -static -Wl,-verbose -T
                      ${CMAKE_CURRENT_SOURCE_DIR}/sloader_static.map)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake
               ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)
