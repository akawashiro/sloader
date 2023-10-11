#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)

if [[ ! -d glibc ]]
then
    git clone git@github.com:akawashiro/glibc.git
    
    pushd glibc
    git remote add upstream https://sourceware.org/git/glibc.git
    git fetch --all
    git checkout origin/print-for-sloader
    popd
fi

mkdir -p glibc-build
mkdir -p glibc-install
cd glibc-build
PREFIX_PATH=$(realpath ../glibc-install)
LD_LIBRARY_PATH="" $(git rev-parse --show-toplevel)/glibc/configure CC="ccache gcc" --prefix=${PREFIX_PATH}
