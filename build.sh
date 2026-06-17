#!/bin/bash

set -e

mkdir -p bin

# Precompile the heavy header once
clang++ -std=c++20 -x c++-header src/dagr.h -o src/dagr.h.pch

time clang++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic\
    -O0\
    -include-pch src/dagr.h.pch \
    src/*.cpp \
    -lcrypto \
    -o bin/dagr

mkdir -p test

cp bin/dagr "test/dagr"

echo -e "\nBuild successful!"