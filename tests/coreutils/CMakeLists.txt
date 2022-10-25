cmake_minimum_required(VERSION 3.16)

add_test(
  NAME coreutils_cat
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_cat.py ${CMAKE_BINARY_DIR}/sloader)

add_test(
  NAME coreutils_objdump
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_objdump.py ${CMAKE_BINARY_DIR}/sloader)

add_test(
  NAME coreutils_readelf
  COMMAND
    python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_readelf.py ${CMAKE_BINARY_DIR}/sloader)
