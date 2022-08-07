import subprocess
import sys

comp = subprocess.run(sys.argv[1:], capture_output=True)
stdout = comp.stdout.decode("utf-8")
succeeded = "Hello World! from fuga" in stdout and "Hello World! from hoge" in stdout

if succeeded:
    sys.exit(0)
else:
    sys.exit(1)
