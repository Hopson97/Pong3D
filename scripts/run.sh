#!/bin/bash

if [ "$1" = "release" ]
then
    ./bin/release/Pong3D 
else
    ./bin/debug/Pong3D 
fi