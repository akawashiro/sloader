# sloader
`sloader` is an ELF loader which aims to replace `ld-linux.so` of glibc.

## How to build & run tests
```
mkdir build
cd build
cmake .. -G Ninja
ninja
ctest -V
```

## Test using custom build glibc
```
./clone_glibc.sh
./build_glibc.sh
SLOADER_LIBRARY_PATH=/home/akira/sloader/glibc-install/lib ./build/sloader ./build/tests/hello_glibc/hello_glibc
```

## TODO
- Speed
- Tests
    - Change ld-linux.so in Docker container
- Link libc statically
