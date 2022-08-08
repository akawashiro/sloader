import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print("cmd:", sys.argv[1:])
stdout = comp.stdout.decode("utf-8")
succeeded = "Hello from libc!" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
