#!/usr/bin/env sh

ACCEPTED_UBUNTU_CODENAMES="focal
bionic
xenial"

printf "%s" "$ACCEPTED_UBUNTU_CODENAMES" | grep -qw "$1"
if [ $? -ne 0 ]; then
    echo "'$1' is not a distribution supported by CMake PPA" 1>&2
    exit 1
fi

printf "Adding CMake PPA for %s\\n" "$1"

# Exit immediately if one of the commands fail below
trap exit ERR

# Commands taken from https://apt.kitware.com/

sudo apt-get update -qq
sudo apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common wget

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $1 main"
sudo apt-get update -qq

echo "CMake repository added successfully"
