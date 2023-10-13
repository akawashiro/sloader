#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)/glibc-build
if which bear; then
    LD_LIBRARY_PATH="" bear -- make all -j 20
    LD_LIBRARY_PATH="" bear -- make install -j 20
else
    LD_LIBRARY_PATH="" make all -j $(nproc)
    LD_LIBRARY_PATH="" make install -j $(nproc)
fi
