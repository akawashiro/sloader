#! /bin/bash -eux

cd $(git rev-parse --show-toplevel)
SLOADER_PATH=$(realpath ./build/sloader)

if [ ! -f ${SLOADER_PATH} ]
then
    echo "sloader not found at ${SLOADER_PATH}"
    exit 1
fi

rm -rf build2
mkdir -p build2
cd build2
${SLOADER_PATH} cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .. -B . -D CMAKE_C_COMPILER_LAUNCHER=${SLOADER_PATH}  -D CMAKE_CXX_COMPILER_LAUNCHER=${SLOADER_PATH} -D CMAKE_C_LINKER_LAUNCHER=${SLOADER_PATH} -D CMAKE_CXX_LINKER_LAUNCHER=${SLOADER_PATH}
${SLOADER_PATH} make VERBOSE=1
${SLOADER_PATH} ctest
