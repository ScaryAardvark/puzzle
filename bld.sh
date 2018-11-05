#!/usr/bin/env bash

[[ ! -d build ]] && mkdir -p build

cd build && cmake .. && make -j 10 

# g++ -Wall -Wpedantic -O6 -fno-exceptions  *.cpp -o puzzle -lpthread -ltbb
# g++ -DINTERNAL_WORD -Wall -Wpedantic -O6 -fno-exceptions  *.cpp -o npuzzle -lpthread -ltbb
