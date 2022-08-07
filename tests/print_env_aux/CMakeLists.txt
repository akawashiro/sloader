cmake_minimum_required(VERSION 3.16)

add_executable(print_env_aux print_env_aux.c)
target_link_options(print_env_aux PUBLIC -static)

add_test(
  NAME print_env_aux
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run.py ${CMAKE_BINARY_DIR}/sloader
    ${CMAKE_BINARY_DIR}/tests/print_env_aux/print_env_aux)
