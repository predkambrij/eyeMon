#!/bin/bash

set -o xtrace
set -o errexit
set -o pipefail


scriptDir="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"


# opencv dependencies
apt-get install -y libc6-i386
apt-get install -y lib32stdc++6 lib32z1 lib32ncurses6 # lib32bz2-1.0
apt-get install -y libc6-i386 lib32stdc++6 lib32gcc-s1 lib32ncurses6 lib32z1
apt-get install -y usbutils unzip g++ git beep
# compiling opencv
apt-get install -y build-essential cmake libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev v4l-utils libv4l-dev # v4l2ucp
# utilities - google test
apt-get install -y libgtest-dev wget


