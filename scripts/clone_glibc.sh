#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)

if [[ $# -ne 1 ]]
then
    echo "Usage: $0 <glibc-version>"
    exit 1
fi
GLIBC_VERSION=$1

if [[ ! -d glibc ]]
then
    git clone --depth 1 --no-single-branch https://github.com/akawashiro/glibc.git
    
    pushd glibc
    git remote add upstream https://sourceware.org/git/glibc.git
    git fetch --all
    git checkout ${GLIBC_VERSION}
    popd
fi

mkdir -p glibc-build
mkdir -p glibc-install
cd glibc-build
PREFIX_PATH=$(realpath ../glibc-install)
LD_LIBRARY_PATH="" $(git rev-parse --show-toplevel)/glibc/configure CC="ccache gcc" --prefix=${PREFIX_PATH}
