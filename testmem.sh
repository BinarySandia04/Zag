#!/bin/bash
mkdir build
cd build
cmake ../
cmake --build .
cd ..

valgrind --leak-check=full build/src/That examples/test.that 