#!/bin/bash
cd "$(dirname "$0")"

mkdir .build 2>/dev/null
cd .build
cmake ../
cmake --build . -j 
cd ..

mkdir -p ~/.that/sources/_Internal
cp -r * ~/.that/sources/_Internal/.
