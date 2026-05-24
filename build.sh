#!/bin/bash

set -e

time clang++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    src/*.cpp \
    -o dagr

echo -e "\nBuild successful!"
