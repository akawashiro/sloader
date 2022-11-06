#!/bin/bash -u

COMMANDS_TO_RUN=$(cat ./ubuntu_usr_bin_commands | grep -v bus | grep -v linux | grep -v ctl | grep -v mtr | grep -v sh | grep -v ".*bin/[nipxkX].*" | grep -v sys | grep -v last | grep -v join | grep -v less)

for c in ${COMMANDS_TO_RUN}; do
    is_dyn=$(readelf -h ${c} 2>&1 | grep DYN | wc -l)
    if [ ${is_dyn} = "1" ]; then
        fs=$(GLOG_logtostderr=1 ./build/sloader ${c} --help 2>&1 | grep --text "Cannot find" | awk -e '{print $7}')
        for f in ${fs}; do
            echo \{\"${f}\", reinterpret_cast\<Elf64_Addr\>\(\&${f}\)\},
        done
    fi
done
