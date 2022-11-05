#! /bin/bash -u

COMMANDS_TO_RUN=$(find /usr/bin -maxdepth 1 -type f | sort | grep -v mm2gv | grep -v zjsdecode | grep -v kerneloops-submit | grep -v ssh-keygen | grep -v xmag | grep -v nop | grep -v akregatorstorageexporter | grep -v bcomps | grep -v bitmap | grep -v breeze-settings5 | grep -v bluetoothctl | grep -v ccomps | grep -v cpufreq-selector | grep -v busctl | grep -v csdpcert | grep -v ctwill-refsort | grep -v display-im6.q16 | grep -v eqn | grep -v erb | grep -v es2gears_wayland.x86_64-linux-gnu | grep -v es2gears_x11.x86_64-linux-gnu | grep -v esc-m | grep -v fcitx | grep -v gc | grep -v linux | grep -v bus | grep -v gt | grep -v gv | grep -v ctl | grep -v kde | grep -v gnome | grep -v lefty | grep -v daemonize | grep -v lastlog | grep -v mtr | grep -v last | grep -v l2 | grep -v laptop | grep -v nslookup | grep -v lava | grep -v kwrite | grep -v kwin | grep -v kw | grep -v kup | grep -v oxygen | grep -v ktrash | grep -v pkaction | grep -v pk | grep -v plasma | grep -v ktel | grep -v ".*bin/k.*" | grep -v plget | grep -v ".*bin/x.*" | grep -v ".*bin/X.*" | grep -v ".*bin/J.*" | grep -v scc | grep -v juk | grep -v scan | grep -v monitor | grep -v julia | grep -v tex | grep -v sh | grep -v tred | grep -v jq | grep -v system | grep -v pass | grep -v jp | grep -v ctrl | grep -v 411 | grep -v ldrdf | grep -v jvf)

for c in ${COMMANDS_TO_RUN}; do
    is_dyn=$(readelf -h ${c} 2>&1 | grep DYN | wc -l)
    if [ ${is_dyn} = "1" ]; then
        fs=$(GLOG_logtostderr=1 ./build/sloader ${c} --help 2>&1 | grep --text "Cannot find" | awk -e '{print $7}')
        for f in ${fs}; do
            echo \{\"${f}\", reinterpret_cast\<Elf64_Addr\>\(\&${f}\)\},
        done
    fi
done
