#! /bin/bash -eux

if [[ ! -d ninja ]]
then
    git clone https://github.com/ninja-build/ninja.git
fi

cmake -B ninja/build-ninja -S ninja
cmake --build ninja/build-ninja -j
