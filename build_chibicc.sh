#! /bin/bash -eux

if [[ ! -d chibicc ]]
then
    git clone https://github.com/rui314/chibicc.git
fi

cd chibicc
make -j
