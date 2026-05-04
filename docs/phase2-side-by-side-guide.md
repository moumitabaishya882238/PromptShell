# Phase 2 — Side-by-side Guide: How llama.cpp, CMake, Ninja and the Build Flow Work

Purpose: teach you, step-by-step, what each tool does, why we use it, exact commands (PowerShell & Git Bash), expected outputs, and troubleshooting tips so you can build and run a local LLM with confidence.

---

## Quick Summary (side-by-side)

| Concept | What it is | Why we need it | Example commands (Windows PowerShell / Git Bash) | Expected result |
|---|---:|---|---|---|
| `git` | Version control client | Download `llama.cpp` source code | `git clone https://github.com/ggerganov/llama.cpp.git E:\PromptShell\llama-cpp` or `git clone https://github.com/ggerganov/llama.cpp.git /e/PromptShell/llama-cpp` | A folder `llama-cpp/` with source files |
| `CMake` | Cross-platform build generator | Produces build files for your compiler (generators: Ninja, MSVC, Makefiles) so you don't type compile commands manually | `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release` | `build/` contains Ninja files and a compile plan |
| `Ninja` | Fast low-level build tool | CMake will call Ninja to run compiles and link steps in parallel | `cmake --build build -j 4` | Compiles source into binaries under `build/` |
| `g++` / `cl` | The compiler (translates C++ → machine code) | Performs actual translation of .cpp → .o → .exe | `g++ --version` or use via CMake | `*.o`, `*.exe` files are produced |
| `llama.cpp` | Local inference engine (C/C++) | Runs the GGUF model locally for offline LLM inference | Build produces an executable (often `main` or `main.exe`) and tools in `build/` | Run the binary with `-m models/model.gguf -p "Hello"` |
| `GGUF model` | Pre-trained model weights in GGUF format | The model's parameters — the LLM brain — required for inference | Put model in `E:\PromptShell\models\model.gguf` and set `MODEL_PATH` in `.env` | The engine loads weights and returns text responses |

---

## What CMake actually does (plain English)

- CMake reads `CMakeLists.txt` files in the project (the project's build recipe).
- It generates a set of build files for the chosen "generator" (Ninja, Visual Studio, Makefiles, etc.).
- CMake does not compile code itself — it prepares files (project files, build.ninja, solution files) that the low-level build tool (Ninja/MSBuild/make) uses to compile.
- Benefits: one recipe works on Windows, macOS, Linux; automatic handling of include dirs, compile flags, libraries.

Analogy: CMake = recipe writer; Ninja = the chef; g++/cl = the cooking tool.

---

## Full step-by-step: Clone → Build → Test (copyable commands)

Notes:
- Use **PowerShell** or **Git Bash**. For Git Bash paths use `/e/PromptShell` style.
- After installing `CMake` or `Ninja`, **restart your terminal** so PATH updates take effect.

1) Clone (done already)

PowerShell:
```powershell
cd E:\PromptShell
git clone https://github.com/ggerganov/llama.cpp.git E:\PromptShell\llama-cpp
```

Git Bash:
```bash
cd /e/PromptShell
git clone https://github.com/ggerganov/llama.cpp.git llama-cpp
```

2) Prepare the build (configure stage)

Open a fresh terminal (important after installs) and run inside the `llama-cpp` folder:

PowerShell:
```powershell
cd E:\PromptShell\llama-cpp
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Git Bash:
```bash
cd /e/PromptShell/llama-cpp
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

What this does:
- `-S .` → source directory
- `-B build` → out-of-source build into `build/`
- `-G Ninja` → tell CMake to create Ninja build files (fast, parallel)
- `-DCMAKE_BUILD_TYPE=Release` → compile optimized release binaries

Common configure errors and fixes:
- `Could not find a generator named Ninja`: install `ninja` and restart shell
- `CMake Error: Could not create named generator`: ensure CMake version is recent (you have 4.3.1)

3) Build (compilation)

PowerShell / Git Bash:
```bash
cmake --build build -j 4
```

What this does:
- Runs the build tool (Ninja) to compile all sources and link the binary
- `-j 4` requests 4 parallel jobs (adjust to CPU cores)

If build fails:
- Read the first compiler error (often missing deps or path issues)
- If `cl` not found: you tried MSVC target — switch to MSYS2/MinGW toolchain or install Visual Studio Build Tools
- If `g++` errors: confirm your PATH contains the MSYS2 `g++` you verified earlier

4) Locate the binary and run a basic test

After success you should have an executable under `build/` — name may vary (check `build/` content). Example run (adjust binary name):

PowerShell (example):
```powershell
# Example binary name: ./build/main.exe  (the actual name may differ)
.
cd E:\PromptShell\llama-cpp\build
# Example run (replace main.exe with actual binary name, and model path with your model file)
./main.exe -m ..\..\models\small.gguf -p "Explain a for-loop in C++"
```

Git Bash (example):
```bash
cd /e/PromptShell/llama-cpp/build
./main -m ../../models/small.gguf -p "Explain a for-loop in C++"
```

Expected output: the program initializes the model and prints a generated response.

5) If you don't have a model yet

- Download a small GGUF test model (llama.cpp README references example models). Place it in `E:\PromptShell\models\` and name it `small.gguf` (or use the real filename you downloaded).
- Update `.env` or pass `-m` with the model path

---

## Example: minimal `.env` usage

Edit `E:\PromptShell\.env` (or copy `.env.example` to `.env`) and set:

```
MODEL_PATH=E:\PromptShell\models\small.gguf
TEMPERATURE=0.7
TOP_P=0.9
MAX_TOKENS=500
SYSTEM_PROMPT=You are PromptShell, a helpful C++ coding assistant focused on beginners.
```

This file is for the higher-level C++ assistant later — llama.cpp binaries take flags; your bridge (Node.js) will read `.env`.

---

## Troubleshooting (common issues)

- `cmake: command not found` → restart terminal or add CMake to PATH
- `ninja: command not found` → restart terminal after winget install, or install ninja manually and add to PATH
- `git clone` failing with timeout → check internet / proxy / VPN or download ZIP from GitHub and extract to `llama-cpp/`
- Build errors about missing headers/libs → read the top build error; often solved by installing required dev tools (MSYS2 packages or Visual Studio Build Tools)

---

## Why we use prompt engineering (not fine-tuning) at Phase 2

- Fast iteration: prompts steer a pre-trained model immediately with no retraining time.
- Low cost: no extra compute, no dataset curation.
- Later (Phase 14) we can add RAG to incorporate repository context without fine-tuning.
- Fine-tuning: reserved for specialized behavior when prompts+RAG can't deliver the expected persona.

---

## Next steps after this guide
1. Confirm `cmake` configure step succeeds (`cmake -S . -B build -G Ninja ...`).
2. Run `cmake --build build -j <cores>` and share any build errors.
3. Download a small GGUF model and run the produced binary with `-m` and a prompt.

---

## Files I created/edited for you
- `docs/procedure.md` (tracked steps)
- `docs/phase2-side-by-side-guide.md` (this file)

---

If you'd like, I can now:
- (A) Run a quick environment check to see why your configure step returned exit code 1, or
- (B) Provide exact commands to download a small GGUF test model and where to get it.

Tell me which option and I'll continue.
