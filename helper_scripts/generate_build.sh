#!/bin/bash

cmake -B build \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

ln -sf build/compile_commands.json compile_commands.json
