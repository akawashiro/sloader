import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print(comp)
print("cmd: ", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8")
print("stdout:")
print(stdout)
succeeded = "AT_RANDOM" in stdout and "PATH" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
