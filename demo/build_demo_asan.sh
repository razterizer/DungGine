#!/bin/bash

clang++ -fsanitize=address -g -O2 demo.cpp -o bin/demo_asan -std=c++20 -I../..
