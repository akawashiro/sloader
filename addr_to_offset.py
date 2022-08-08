#! python3

import argparse
import pathlib
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('map_file', type=pathlib.Path)
parser.add_argument('address', type=str)

args = parser.parse_args()
if args.address[0:2] == "0x":
    address = int(args.address, 16)
else:
    address = int(args.address)

with open(args.map_file) as f:
    for line in f.readlines():
        fields = line.split(" ")
        filename = fields[0]
        file_start = int(fields[1][:18], 16)
        file_end = int(fields[1][19:37], 16)
        mem_start = int(fields[5][:18], 16)
        mem_end = int(fields[5][19:37], 16)

        if mem_start <= address and address <= mem_end:
            print(filename, ":", hex(address - mem_start + file_start))
            addr2line_cmd = ["addr2line", "-e", filename, hex(address - mem_start + file_start)]
            comp = subprocess.run(addr2line_cmd, capture_output=True)
            print(comp.stdout.decode("utf-8"), end="")
