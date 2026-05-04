# PromptShell

A hybrid AI system combining C++ backend + VS Code extension + Local LLM for AI-powered coding assistance.

## Project Vision

- Phase 1-3: Master C++ from basics to advanced
- Phase 4+: Expand to multiple languages
- Complete offline operation with local LLM

## Tech Stack

- **Backend**: C++ (OOP)
- **LLM Engine**: llama.cpp
- **Bridge**: Node.js
- **Frontend**: VS Code Extension (TypeScript)

## Current Phase

**PHASE 11: CLI activation / bridge** 🔄

## Quick Start

See [docs/procedure.md](docs/procedure.md) for development progress and architecture decisions.

## CLI Usage

After building the backend, you can run PromptShell directly:

```bash
cd /e/PromptShell/backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
./build/bin/promptshell.exe activate
```

Useful commands:

```bash
./build/bin/promptshell.exe activate
./build/bin/promptshell.exe --prompt="What is a pointer in C++?"
./build/bin/promptshell.exe --run-file=tests/xyz.cpp
./build/bin/promptshell.exe --mode=json --assistant-mode=code-only --prompt="Explain std::vector"
```

To make it available from anywhere, install it into a PATH directory:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install.ps1
```

Or from Git Bash / MSYS2:

```bash
bash scripts/install.sh
```

## Architecture

```
User Input (Terminal/VS Code)
    ↓
VS Code Extension / Terminal UI
    ↓
Node.js Bridge (child_process)
    ↓
C++ Assistant Backend (OOP Classes)
    ↓
llama.cpp (Local LLM Inference)
    ↓
Structured Response
```

## License

MIT
