#!/bin/bash

set -e

time clang++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic\
    -O0\
    src/*.cpp \
    -o dagr

cp dagr "test/dagr"

echo -e "\nBuild successful!"
