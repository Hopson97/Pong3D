#!/bin/bash

sh scripts/build.sh release 

rm -rf -d Pong3D

mkdir Pong3D

cp bin/release/Pong3D Pong3D
cp -r res Pong3D

echo "Deploy build created."
echo "cd Pong3D to find it"