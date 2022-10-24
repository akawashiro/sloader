import subprocess
import sys
import os

comp = subprocess.run([sys.argv[1], "/bin/objdump", "-h", "/bin/objdump"], capture_output=True)
print(comp)
print("cmd:", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8")
print("stdout:")
print(stdout)

with open(os.path.abspath(__file__)) as file:
    content = file.read()

succeeded = ".text" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
