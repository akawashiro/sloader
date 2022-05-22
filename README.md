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
