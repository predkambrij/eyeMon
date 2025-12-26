#!/bin/bash

set -o xtrace
set -o errexit
set -o pipefail


scriptDir="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"

# http://fabiorehm.com/blog/2014/09/11/running-gui-apps-with-docker/
apt-get install -y x11-apps sudo
# Replace with your user / group id
export uid=$ARG_UID gid=$ARG_GID
mkdir -p /home/developer
echo "developer:x:${uid}:${gid}:Developer,,,:/home/developer:/bin/bash" >> /etc/passwd
echo "developer:x:${uid}:" >> /etc/group
echo "developer ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/developer
chmod 0440 /etc/sudoers.d/developer
chown ${uid}:${gid} -R /home/developer
/bin/bash -c "echo 'developer:developerpw' | chpasswd"
usermod -a -G video developer
usermod -a -G audio developer
# ssh https://docs.docker.com/examples/running_ssh_service/
#   and SSH login fix. Otherwise user is kicked off after login
apt-get install -y openssh-server
mkdir /var/run/sshd
