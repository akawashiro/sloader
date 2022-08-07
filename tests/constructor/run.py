import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print(comp)
stdout = comp.stdout.decode("utf-8")
succeeded = "====== Before main ======" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
