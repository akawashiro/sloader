import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print(comp)
print("cmd:", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8", errors="ignore")
print("stdout:")
print(stdout)
succeeded = "Hello using write(2)" in stdout and "Hello using printf(3)" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
