#!/bin/bash

cmake --build build -j$(nproc) && ./build/game_examples/cube_platformer
