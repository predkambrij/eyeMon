#!/bin/bash

set -o xtrace
set -o errexit
set -o pipefail


scriptDir="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"

apt-get update
apt-get upgrade -y

echo "Europe/Ljubljana" > /etc/timezone
dpkg-reconfigure -f noninteractive tzdata

apt-get install -y pwgen python-pip

