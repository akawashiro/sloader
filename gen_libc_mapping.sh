#! /bin/bash -eu

COMMAND_TO_RUN="htop -h"
fs=$(GLOG_logtostderr=1 ./build/sloader ${COMMAND_TO_RUN} 2>&1 | grep --text "Cannot find" | awk -e '{print $7}')

for f in ${fs}; do
    echo \{\"${f}\", reinterpret_cast\<Elf64_Addr\>\(\&${f}\)\},
done
