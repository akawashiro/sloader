add_executable(hello_static_glibc hello.c)
target_link_libraries(hello_static_glibc -static)

add_test(
  NAME hello_static_glibc_test
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/hello_static_glibc/hello_static_glibc)
