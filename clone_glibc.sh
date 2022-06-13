#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)
git clone https://github.com/akawashiro/glibc.git

pushd glibc
git fetch --all
git checkout "2.34"
popd

mkdir -p glibc-build
mkdir -p glibc-install
cd glibc-build
PREFIX_PATH=$(realpath ../glibc-install)
LD_LIBRARY_PATH="" $(git rev-parse --show-toplevel)/glibc/configure CC="ccache gcc" --prefix=${PREFIX_PATH}
