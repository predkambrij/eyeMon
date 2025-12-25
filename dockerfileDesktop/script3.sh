#!/bin/bash

set -o xtrace
set -o errexit
set -o pipefail


scriptDir="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"


# opencv dependencies
apt-get install -y libc6-i386
apt-get install -y lib32stdc++6 lib32z1 lib32ncurses5 lib32bz2-1.0
apt-get install -y libc6-i386 lib32stdc++6 lib32gcc1 lib32ncurses5 lib32z1 
apt-get install -y usbutils
apt-get install -y unzip
apt-get install -y g++ git beep
# compiling opencv
apt-get install -y build-essential cmake libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev v4l2ucp v4l-utils libv4l-dev
# utilities - google test
apt-get install -y libgtest-dev wget
wget https://github.com/Itseez/opencv/archive/2.4.10.4.zip -O /2.4.10.4.zip
bash -c "echo /usr/local/lib >> /etc/ld.so.conf; ldconfig"

