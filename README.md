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

**PHASE 1: Project Setup** ✅

## Quick Start

See [docs/procedure.md](docs/procedure.md) for development progress and architecture decisions.

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
