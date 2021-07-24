add_executable(sloader sloader.cc)
target_link_libraries(sloader glog)

add_custom_target(hello_asm ALL
    COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/hello.asm -o ${CMAKE_CURRENT_BINARY_DIR}/hello.o && ld -s -o hello hello.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
