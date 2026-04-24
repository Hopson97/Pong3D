#!/bin/bash

target_release() {
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../..
    make
    echo "Built target in build/release/"
    cd ../..
}

target_debug() {
    cd debug 
    cmake -DCMAKE_BUILD_TYPE=Debug ../..
    make
    echo "Built target in build/debug/"
    cd ../..
}

# Create folder for distribution
if [ "$1" = "release" ]
then
    if [ -d "$pong-3d" ]
    then
        rm -rf -d pong-3d
    fi

    mkdir -p pong-3d
fi

# Creates the folder for the buildaries
mkdir -p pong-3d 
mkdir -p pong-3d/assets
mkdir -p build
mkdir -p build/release
mkdir -p build/debug
cd build

# Builds target
if [ "$1" = "release" ]
then
    target_release
    cp build/release/pong-3d pong-3d/pong-3d
else
    target_debug
fi

cp -R assets pong-3d/
