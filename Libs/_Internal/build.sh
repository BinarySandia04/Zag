#!/bin/bash
cd "$(dirname "$0")"

mkdir .build 2>/dev/null
cd .build
cmake ../
cmake --build . -j 
cd ..

mkdir -p ~/.zag/sources/_Internal
cp -r * ~/.zag/sources/_Internal/.
