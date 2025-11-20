#
#  Install Docker on supported systems if not previously installed.
#
#  Docker is used in the ADAN project to avoid installing unwanted programs
#   on the host machine. It is also used to maintain a Linux/Unix
#   environment regardless of what kernel your operating system is using.
#

import subprocess

def has_docker() -> bool:
    result = subprocess.run(["docker", "--version"], capture_output=True, text=True)
    return "" == result.stderr

def install():
    if True == has_docker():
        print("Docker is already installed on your system.")
    else:
        #
        #  We can figure out what operating system we're using simply by checking if
        #   it supports the Bash scripting language or not.
        #
        def uses_bash() -> bool:
            result = subprocess.run(["bash", "--version"], capture_output=True, text=True)
            return "" == result.stderr

        if False == uses_bash():
            def has_chocolatey() -> bool:
                result = subprocess.run(["choco", "-v"], capture_output=True, text=True)
                return "" == result.stderr

            #
            #  Silently install Chocolatey to circumvent user
            #   confirmation. (For fully automatic installation)
            #
            if False == has_chocolatey():
                subprocess.run([
                    "powershell",
                    "-NoProfile",
                    "-ExecutionPolicy", "Bypass",
                    "-Command",
                    "[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; "
                    "iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
                ])
            
            #
            #  Use the Chocolatey CLI to install Docker without a
            #   popup executable. (Fully automatic installation)
            #
            subprocess.run([
                "powershell",
                "-Command",
                "choco install docker-desktop -y"
            ])
        else:
            print("Installing Docker ...")

            subprocess.run(["curl", "-fsSL", "https://get.docker.com", "-o", "get-docker.sh"])
            subprocess.run(["chmod", "+x", "./get-docker.sh"])
            subprocess.run(["sudo", "./get-docker.sh"])
            subprocess.run(["rm", "get-docker.sh"])

            print("Docker has been installed on your system.")

install()