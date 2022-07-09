#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)/glibc-build
LD_LIBRARY_PATH="" bear -- make all -j 20
LD_LIBRARY_PATH="" bear -- make install -j 20
