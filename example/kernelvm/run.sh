#!/bin/bash -eux
cat ./hoge.c
cat ./hello.c
gcc -o libhoge.so -fPIC -shared hoge.c
gcc -o hello -fPIC hello.c libhoge.so
./hello
ldd libhoge.so
ldd hello
