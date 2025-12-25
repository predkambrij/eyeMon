#!/bin/bash

set -o xtrace
set -o errexit
set -o pipefail


scriptDir="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"

cd /home/developer/ && sudo chmod a+r /2.4.10.4.zip && unzip /2.4.10.4.zip
cd opencv-* && mkdir release && cd release
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j8 && sudo make install
sudo ldconfig
rm -rf /home/developer/opencv-*
