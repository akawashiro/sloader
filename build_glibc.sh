#! /bin/bash -eux

pushd $(git rev-parse --show-toplevel)/glibc-build
LD_LIBRARY_PATH="" bear -- make all -j 20
LD_LIBRARY_PATH="" make install -j 20
popd
cp $(git rev-parse --show-toplevel)/glibc-build/compile_commands.json $(git rev-parse --show-toplevel)/glibc
