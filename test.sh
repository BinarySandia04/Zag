#!/bin/bash
mkdir build
cd build
cmake ../
cmake --build .
cd ..

build/src/That examples/test.that