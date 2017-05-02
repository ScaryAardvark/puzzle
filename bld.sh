#!/usr/bin/env bash

g++ -Wall -Wpedantic -O6 -fno-exceptions  *.cpp -o puzzle -lpthread -ltbb
