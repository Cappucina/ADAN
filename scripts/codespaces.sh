#!/bin/bash
# 
#  This script's intended environment is strictly GitHub Codespaces. The version of
#   Codespaces this script was written on is: `6.8.0-1030-azure`.
# 
#  Major utility and quality of life script written to make your life a billion
#   times easier.
#
#  ~ This script supports arguments, use them like: `./codespaces.sh <arg>`.
#
##
# 
#  Features:
#   - Setup Docker
# 

if [ -z "$1" ]; then
    echo "Usage: $0 <command>"
    exit 1
fi

#### Options ####

shopt -s nocasematch

#### Commands ####

if [[ "$1" == "setup" ]]; then
    sudo apt-get update
    sudo apt-get remove docker docker-engine docker.io containerd runc
    
    python3 /workspaces/ADAN/scripts/container.py

    sudo apt-get update
    sudo apt-get install ca-certificates curl gnupg
    sudo install -m 0755 -d /etc/apt/keyrings
    
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
        echo \
        "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
        https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
        sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
fi