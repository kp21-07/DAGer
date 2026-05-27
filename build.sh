#!/bin/bash

set -e

mkdir -p bin

time clang++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic\
    -O0\
    ./*.cpp \
    -lcrypto \
    -o bin/dagr

mkdir -p test

cp bin/dagr "test/dagr"

echo -e "\nBuild successful!"