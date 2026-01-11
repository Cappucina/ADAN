import os
import subprocess

for root, dirs, files in os.walk("."):
    if "build" in root or ".git" in root:
        continue
    for file in files:
        if file.endswith((".c", ".h")) and ("source" in root or "include" in root):
            path = os.path.join(root, file)
            subprocess.run(["clang-format", "-i", path])
