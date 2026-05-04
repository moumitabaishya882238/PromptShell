@echo off
REM Launch MSYS2 MinGW64 shell and rebuild llama.cpp
C:\msys64\mingw64.exe bash -c "cd /e/PromptShell/llama-cpp && rm -rf build && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 4"
pause
