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
target_link_options(sloader PUBLIC -static -T
                    ${CMAKE_CURRENT_SOURCE_DIR}/sloader.map)
target_compile_options(sloader PUBLIC -Wall -Werror)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake
               ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)
