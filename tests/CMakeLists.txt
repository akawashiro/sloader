cmake_minimum_required(VERSION 3.16)

add_custom_target(
  hello_asm ALL
  COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/hello.asm -o
          ${CMAKE_CURRENT_BINARY_DIR}/hello.o && ld -s -o hello_asm hello.o
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(
  NAME hello_asm_test
  COMMAND ./sloader ./tests/hello_asm
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Hand written testcases
add_subdirectory(print_env_aux)
add_subdirectory(cascades)
add_subdirectory(find_a_depending_so)
add_subdirectory(hello_glibc)
add_subdirectory(hello_static_glibc)
add_subdirectory(global_variable)
add_subdirectory(static_struct)
add_subdirectory(constructor)
add_subdirectory(local_exec)
add_subdirectory(local_dynamic)
add_subdirectory(generic_dynamic)
add_subdirectory(initial_exec)
add_subdirectory(ls)
add_subdirectory(print_args)
add_subdirectory(errno)
add_subdirectory(dlopen)
add_subdirectory(dlopen2)

# GNU Coreutils tests
add_test(
  NAME coreutils_cat
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/cat
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_cksum
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/cksum
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_sha1sum
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/sha1sum
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_md5sum
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/md5sum
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_od
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
          ${CMAKE_BINARY_DIR}/sloader /bin/od ${CMAKE_BINARY_DIR}/sloader)

add_test(
  NAME coreutils_head
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/head
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_sort
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/sort
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_uniq
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/uniq
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(
  NAME coreutils_wc
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/wc
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(NAME coreutils_date
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/date)

add_test(NAME coreutils_who
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/who)

add_test(
  NAME coreutils_stat
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/stat
    ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py)

add_test(NAME coreutils_hostid
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/hostid)

add_test(NAME coreutils_printenv
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/printenv)

add_test(NAME coreutils_id
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/id)

add_test(NAME coreutils_logname
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/logname)

add_test(NAME coreutils_uname
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /bin/uname -a)

# GNU Bintuils tests
add_test(
  NAME binutils_objdump
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/objdump -d ${CMAKE_BINARY_DIR}/sloader)

add_test(
  NAME binutils_readelf
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /bin/readelf -a ${CMAKE_BINARY_DIR}/sloader)

# My daily use tools
add_test(NAME test_ps
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /usr/bin/ps --help)

add_test(NAME test_top
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /usr/bin/top -h)

add_test(NAME test_tmux
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader tmux --help)

# add_test(NAME test_nvim COMMAND python3
# ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py ${CMAKE_BINARY_DIR}/sloader nvim
# --help)

add_test(NAME test_python3
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader /usr/bin/python3 --help)

add_test(NAME test_gcc
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader gcc --help)

add_test(NAME test_make
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader make --help)

add_test(NAME test_zsh
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader zsh --help)

add_test(NAME test_rustc
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
                 ${CMAKE_BINARY_DIR}/sloader rustc --help)

add_test(
  NAME test_python3_hello
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /usr/bin/python3 ${CMAKE_CURRENT_SOURCE_DIR}/hello.py)

add_test(
  NAME test_python3_ctypes
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
    ${CMAKE_BINARY_DIR}/sloader /usr/bin/python3 ${CMAKE_CURRENT_SOURCE_DIR}/ctypes.py)

# Installing additional packages is required but not supported yet.
#
# Just `python3` may be a bash script because of pyenv. So, we must use
# `/usr/bin/python3` instead. Then, we must install torch system-wide. But, I
# don't want to install torch system-wide just for testing.
# add_test(
#   NAME test_python3_torch_ones
#   COMMAND
#     python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_stdout.py
#     ${CMAKE_BINARY_DIR}/sloader /usr/bin/python3
#     ${CMAKE_CURRENT_SOURCE_DIR}/torch_ones.py)

add_test(
  NAME test_bash_plus
  COMMAND
    python3 ${cmake_current_source_dir}/compare_stdout.py
    ${cmake_binary_dir}/sloader bash
    ${cmake_current_source_dir}/one_plus_one.sh)

add_test(NAME test_curl
         COMMAND python3 ${cmake_current_source_dir}/compare_stdout.py
                 ${cmake_binary_dir}/sloader curl -s www.example.com)

set_tests_properties(test_rustc test_top test_bash_plus test_curl
                     test_python3_torch_ones PROPERTIES WILL_FAIL TRUE)

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
    COMMAND ${CMAKE_BINARY_DIR}/sloader
            ${CMAKE_CURRENT_BINARY_DIR}/hello_static_custom_libc
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
