#!/bin/bash

clang++ -fsanitize=undefined -g -O2 demo.cpp -o bin/demo_ubsan -std=c++20 -I../..
