#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)/glibc-build
LD_LIBRARY_PATH="" make all -j $(($(nproc) - 1))
LD_LIBRARY_PATH="" make install -j $(($(nproc) - 1))
