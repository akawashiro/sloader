#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)
cmakelists=$(find . | grep CMakeLists.txt)

for c in ${cmakelists}; do
    cmake-format -i ${c}
done
