#!/bin/bash -ux
# How to use
# perf_svg.sh ./build/sloader /bin/ls

prefix=perf_svg_$(date +'%Y%m%d%H%M%S')
perf record --freq=max -g --output=${prefix}_perf.data -- $@
if [[ ! -d FlameGraph ]]; then
    git clone https://github.com/brendangregg/FlameGraph
fi
cd FlameGraph
perf script -i ../${prefix}_perf.data | ./stackcollapse-perf.pl > ${prefix}.perf-folded
./flamegraph.pl ${prefix}.perf-folded > ../${prefix}.svg
