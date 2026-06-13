#!/bin/bash

set -e

mkdir -p bin

# Precompile the heavy header once
clang++ -std=c++20 -x c++-header dagr.h -o dagr.h.pch

time clang++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic\
    -O0\
    -include-pch dagr.h.pch \
    ./*.cpp \
    -lcrypto \
    -o bin/dagr

mkdir -p test

cp bin/dagr "test/dagr"

echo -e "\nBuild successful!"