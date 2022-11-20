import subprocess
import sys
import os

comp = subprocess.run(sys.argv[1:], capture_output=True)
print("cmd:", " ".join(sys.argv[1:]))
stdout_sloader = comp.stdout.decode("utf-8")
print(f"{stdout_sloader=}")
stderr_sloader = comp.stderr.decode("utf-8")
print(f"{stderr_sloader=}")

comp = subprocess.run(sys.argv[2:], capture_output=True)
print("cmd:", " ".join(sys.argv[2:]))
stdout_ld = comp.stdout.decode("utf-8")
print(f"{stdout_ld=}")
stderr_ld = comp.stderr.decode("utf-8")
print(f"{stderr_ld=}")

succeeded = (stdout_sloader == stdout_ld)

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
