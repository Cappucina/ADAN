#
#  Create a new Docker container for managing and toying
#   with ADAN. This container will be where all
#   compilation happens, to avoid using up the host's
#   system.
#
#  CONTAINER NAME: "adan-c"
#

from pathlib import Path
from install_docker import has_docker, install

import subprocess

CONTAINER_NAME = "adan-c"

if False == has_docker():
    install()

#
#  Returns the parent of this script's parent (e.g. scripts)
#   to find ADAN's true file path.
#
def find_relative_path():
    script_path = Path(__file__).resolve()
    ancestor = script_path.parent.parent
    return ancestor

subprocess.run(["docker", "build", "-t", CONTAINER_NAME, find_relative_path()])