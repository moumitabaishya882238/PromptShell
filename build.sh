#!/bin/bash
set -e
cd /e/PromptShell/llama-cpp
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
echo "Build complete!"
ls -la build/bin/
