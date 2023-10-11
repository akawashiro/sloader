#! /bin/bash -eux

if [[ ! -d curl ]]
then
    git clone https://github.com/curl/curl.git
fi

cmake -B curl/build -S curl 
cmake --build curl/build -j
