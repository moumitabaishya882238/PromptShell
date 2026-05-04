# PromptShell Development Procedure

## Overview
Building a resume-level AI coding assistant with C++ backend, local LLM integration, and VS Code extension frontend.

---

## PHASE 1: Project Setup ✅

### Objective
Establish the foundational project structure and verify development environment setup.

### What We Did

#### 1. Directory Structure
```
PromptShell/
├── backend/
│   ├── src/           (C++ source files)
│   ├── include/       (C++ headers)
│   ├── build/         (CMake output)
│   └── CMakeLists.txt (Build configuration)
├── llama-cpp/         (llama.cpp repository - to be cloned)
├── models/            (GGUF model storage)
├── bridge/            (Node.js bridge - Phase 11)
├── extension/         (VS Code extension - Phase 12)
├── docs/              (Documentation)
├── .gitignore         (Git ignore rules)
├── .env.example       (Configuration template)
└── README.md          (Project overview)
```

#### 2. Build Configuration
- **File**: `backend/CMakeLists.txt`
- **Purpose**: CMake configuration for compiling C++ backend
- **Key Features**:
  - C++17 standard
  - Auto-detects source files in `src/`
  - Outputs binary to `build/bin/`

#### 3. Configuration Template
- **File**: `.env.example`
- **Purpose**: Template for runtime settings
- **Settings**:
  - Model path (will be populated in Phase 2)
  - LLM inference parameters
  - System prompt for AI behavior

#### 4. Git Configuration
- **File**: `.gitignore`
- **Purpose**: Prevents committing large files and build artifacts
- **Ignores**:
  - Build artifacts (`*.o`, `*.exe`)
  - Large model files (`*.gguf`)
  - IDE configs
  - Dependencies

### Architecture Decisions

| Decision | Reason |
|----------|--------|
| CMake for build system | Cross-platform, industry standard |
| C++17 standard | Balances modern features with compatibility |
| Separate `include/` and `src/` | Professional C++ project structure |
| Environment variables | Easy configuration without hardcoding |

### Next: PHASE 2
- Install C++ compiler (if needed)
- Clone and build llama.cpp
- Download a small GGUF model

### Files Created
1. `backend/CMakeLists.txt` - Build configuration
2. `.gitignore` - Version control filter
3. `.env.example` - Configuration template
4. `README.md` - Project overview

---

## Expected Outcomes After Phase 1
✅ Project structure ready
✅ Build system configured
✅ Git ready for version control
⏳ Ready for Phase 2: LLM setup

---

**Progress**: 1/15 Phases Complete

---

## PHASE 2: Run LLM Manually ✅

### Objective
Get the local LLM engine working outside the extension first, so we can test the AI core in a simple command-line flow.

### What We Did
- Cloned `llama-cpp/` successfully.
- Configured build with CMake + Ninja in MSYS2 MinGW64.
- Resolved Windows/MinGW build compatibility issues.
- Built llama.cpp tools successfully (`build/bin/*.exe`).
- Downloaded and tested a GGUF model with `llama-cli.exe`.

### Verified Outcome
- `llama-cli.exe` loads model and generates response from prompt.
- Local inference is working offline in terminal.

### Learning Note
- `CMake` generates build instructions; `Ninja` executes fast parallel builds.
- `llama.cpp` is the inference runtime; `.gguf` is the model weights file.
- Placeholder model names (example: `your-model.gguf`) must be replaced with real downloaded filenames.

### File Focus
- `llama-cpp/CMakeLists.txt`
- `llama-cpp/build/bin/`
- `models/`

**Progress**: 2/15 Phases Complete

---

## PHASE 3: Basic C++ Input System ✅

### Objective
Build a minimal C++ program that reads user input from the terminal and echoes it back. This is the foundation for the standalone backend before connecting to llama.cpp.

### What We Did
- Created `backend/src/main.cpp` with basic input/output logic.
- Used `std::getline()` to read full prompts from user.
- Updated `backend/CMakeLists.txt` to compile the program.
- Built successfully in MSYS2 MinGW64.
- Tested: program accepts input and displays it.

### Verified Outcome
```
PromptShell - Basic Input System (Phase 3)
============================================

Enter your prompt: Explain a for loop
You entered:
Explain a for loop

✓ Input system working!
```

### Key Concepts
- `std::getline()`: reads full input line (including spaces) from stdin.
- `std::cout`: outputs text to stdout.
- `std::string`: modern C++ text container (safer than char arrays).

### File Focus
- `backend/src/main.cpp` - main entry point
- `backend/CMakeLists.txt` - build rules for backend executable
- `backend/build/bin/promptshell.exe` - compiled binary

### Build Command (MSYS2 MinGW64)
```bash
cd /e/PromptShell/backend
cmake -S . -B build -G Ninja
cmake --build build -j 4
./build/bin/promptshell
```

**Progress**: 3/15 Phases Complete

---

## PHASE 4: Connect C++ with llama.cpp ✅

### Objective
Integrate the user input program with the llama.cpp inference engine. The C++ program now calls llama-cli to generate AI responses.

### What We Did
- Updated `backend/src/main.cpp` to call `llama-cli.exe` via `system()`.
- Built the llama command string dynamically with user prompt.
- Passed model path and user input to the LLM engine.
- Captured and displayed the LLM response in real-time.

### Verified Outcome
```
PromptShell - AI Coding Assistant (Phase 4)
==========================================

Enter your prompt: What is a pointer in C++?

[Processing...]

[LLM Response displays here]

✓ Response generated successfully!
```

### Key Code Pattern
```cpp
std::string command = "llama-cli.exe -m \"model.gguf\" -p \"" + prompt + "\"";
int result = system(command.c_str());
```

### Learning Note
- `system()`: executes shell command from C++
- Command string must properly escape quotes around arguments
- Model path and llama-cli path are hardcoded (will improve in Phase 5)

### File Focus
- `backend/src/main.cpp` - now calls llama-cli with user prompt
- `llama-cpp/build/bin/llama-cli.exe` - the inference engine being called

### Build Command (MSYS2 MinGW64)
```bash
cd /e/PromptShell/backend
cmake -S . -B build -G Ninja
cmake --build build -j 4
./build/bin/promptshell
```

**Progress**: 4/15 Phases Complete

---

## PHASE 5: Master Prompt Integration 🚧
