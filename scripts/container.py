#
#  Create a new Docker container for managing and toying
#   with ADAN. This container will be where all
#   compilation happens, to avoid using up the host's
#   system.
#
#  CONTAINER NAME: "adan-c"
#

from pathlib import Path

import subprocess

CONTAINER_NAME = "adan-dev-container"

#
#  Returns the parent of this script's parent (e.g. scripts)
#   to find ADAN's true file path.
#
def find_relative_path():
	script_path = Path(__file__).resolve()
	ancestor = script_path.parent.parent
	return ancestor

try:
	subprocess.run([
		"docker", "build", "-q", "-t", CONTAINER_NAME,
		"--platform", "linux/amd64", find_relative_path()
	], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except subprocess.CalledProcessError:
	subprocess.run([
		"docker", "build", "-q", "-t", CONTAINER_NAME,
		find_relative_path()
	], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)