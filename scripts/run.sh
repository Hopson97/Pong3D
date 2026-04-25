#!/bin/bash

if [ "$1" = "release" ]
then
    ./build/release/pong-3d
else
    ./build/debug/pong-3d
fi