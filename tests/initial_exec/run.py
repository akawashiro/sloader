import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print("cmd:", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8")
print("stdout:")
print(stdout)
succeeded = "__thread uint32_t i0=aaaaaaaa" in stdout and "__thread uint32_t i1=bbbbbbbb" in stdout and "__thread uint32_t i2=cccccccc" in stdout and "__thread uint32_t i3=dddddddd" in stdout and "Should be 0xaabbccdd: uint32_t j=aabbccdd" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
