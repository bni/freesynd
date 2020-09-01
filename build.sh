#!/bin/bash

set -e

./cmake-3.17.1-Darwin-x86_64/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=release .

make

cp SYND/DATA/* src/FreeSynd.app/Contents/Resources/data/
cp -r data/* src/FreeSynd.app/Contents/Resources/data/
