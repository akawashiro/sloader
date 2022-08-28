import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print("cmd:", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8")
print("comp.stdout: ", comp.stdout)
print("stdout:", stdout)
succeeded = "Hello from libc!" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
