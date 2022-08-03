cmake_minimum_required(VERSION 3.16)

add_executable(print_env_aux print_env_aux.c)
target_link_options(print_env_aux PUBLIC -static)

add_test(
  NAME print_env_aux_test
  COMMAND ./sloader ./tests/print_env_aux
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})


add_executable(hello_static_libc hello.c)
target_link_libraries(hello_static_libc -static)

add_test(
  NAME hello_static_libc_test
  COMMAND ./sloader ./tests/hello_static_libc
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(
  hello_asm ALL
  COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/hello.asm -o
          ${CMAKE_CURRENT_BINARY_DIR}/hello.o && ld -s -o hello_asm hello.o
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(
  NAME hello_asm_test
  COMMAND ./sloader ./tests/hello_asm
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(cascades)

add_subdirectory(find_a_depending_so)

add_subdirectory(hello_glibc)

add_subdirectory(global_variable)

add_subdirectory(static_struct)

add_subdirectory(constructor)

add_subdirectory(local_exec)

add_subdirectory(initial_exec)

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
    COMMAND ${CMAKE_BINARY_DIR}/sloader ${CMAKE_CURRENT_BINARY_DIR}/hello_static_custom_libc
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
