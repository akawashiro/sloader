import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
print("cmd:", " ".join(sys.argv[1:]))
stdout = comp.stdout.decode("utf-8", errors="ignore")
succeeded = "deadbeef" in stdout and "abcdef12" in stdout and "aabbccdd" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
