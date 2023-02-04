#!/bin/bash

if [ "$1" = "release" ]
then
    ./bin/release/bin/Pong3D 
else
    ./bin/debug/bin/Pong3D 
fi