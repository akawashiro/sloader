#! /bin/bash -eux

if [[ ! -d curl ]]
then
    git clone https://github.com/curl/curl.git
fi

rm -rf curl-install
mkdir -p curl-install
cmake -B curl/build -S curl -DCMAKE_INSTALL_PREFIX=curl-install
cmake --build curl/build -- install -j
