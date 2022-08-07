add_executable(hello_static_libc hello.c)
target_link_libraries(hello_static_libc -static)

add_test(
  NAME hello_static_libc_test
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/hello_static_libc)
