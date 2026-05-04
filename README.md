# PromptShell

A **resume-level local AI coding assistant** built with C++, llama.cpp, and a modular OOP architecture. Write code, compile it locally, get instant AI explanations—no cloud APIs, no internet required.

![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-green)
![Status](https://img.shields.io/badge/status-Active%20Development-orange)

---

## 🚀 Quick Start

### Windows (PowerShell)
```powershell
powershell -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/yourusername/PromptShell/main/install.ps1' -OutFile 'install.ps1'; .\install.ps1"
```

Or locally:
```powershell
powershell -ExecutionPolicy Bypass -File install.ps1
```

### Linux & macOS (Bash)
```bash
bash <(curl -sL https://raw.githubusercontent.com/yourusername/PromptShell/main/install.sh)
```

Or locally:
```bash
bash install.sh
```

---

## 📋 Prerequisites

### Windows
- **Git** - [Download](https://git-scm.com/download/win)
- **CMake** - [Download](https://cmake.org/download/)
- **Ninja** - [Download](https://github.com/ninja-build/ninja/releases)
- **MinGW64 (MSYS2)** - [Download](https://www.msys2.org/)
- **PowerShell 5.1+** (built-in on Windows 10+)

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y git cmake ninja-build build-essential
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install git cmake ninja-build gcc-c++
```

### macOS
```bash
brew install git cmake ninja
xcode-select --install  # For g++ compiler
```

---

## 📥 Installation

### Option 1: Automated Installation (Recommended)

#### Windows
1. Open PowerShell as Administrator
2. Run:
```powershell
powershell -ExecutionPolicy Bypass -File install.ps1
```

#### Linux / macOS
1. Open Terminal
2. Run:
```bash
bash install.sh
```

### Option 2: Manual Installation

#### Windows
```powershell
# Clone the repository
git clone https://github.com/yourusername/PromptShell.git
cd PromptShell

# Build the backend
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Download the model
# Visit: https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF
# Save to: PromptShell\models\tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf

# Add to PATH
$BinDir = "$PWD\build\bin"
[System.Environment]::SetEnvironmentVariable("PATH", "$env:PATH;$BinDir", [System.EnvironmentVariableTarget]::User)

# Restart terminal and test
promptshell --help
```

#### Linux / macOS
```bash
# Clone the repository
git clone https://github.com/yourusername/PromptShell.git
cd PromptShell

# Build the backend
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Download the model
mkdir -p models
cd models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
cd ..

# Add to PATH
echo 'export PROMPTSHELL_HOME="'$PWD'"' >> ~/.bashrc
echo 'export PATH="$PATH:$PROMPTSHELL_HOME/build/bin"' >> ~/.bashrc
source ~/.bashrc

# Test
promptshell --help
```

---

## 💻 Usage

### Basic Commands

#### Run a C++ file
```bash
promptshell run test.cpp
```

#### Debug with AI explanation
```bash
promptshell debug test.cpp
```

#### Ask a code-related question
```bash
promptshell ask "explain this function signature"
```

#### Execute shell commands
```bash
promptshell shell "ls -la"
```

#### Show help
```bash
promptshell --help
```

### Example Workflow

1. **Create a C++ file with an error:**
```cpp
// test.cpp
#include <iostream>

int main() {
    std::string name = "Alice";
    std::cout << name.length() << std::endl;  // Missing #include <string>
    return 0;
}
```

2. **Run debug mode:**
```bash
promptshell debug test.cpp
```

3. **Output:**
```
ISSUE: Missing header for std::string
FIX: Add #include <string> at the top
CODE:
#include <iostream>
#include <string>

int main() {
    std::string name = "Alice";
    std::cout << name.length() << std::endl;
    return 0;
}
```

---

## 🏗️ Project Structure

```
PromptShell/
├── backend/
│   ├── src/
│   │   ├── main.cpp              # REPL engine
│   │   ├── promptshell.cpp        # Core orchestration
│   │   ├── llm_engine.cpp         # llama.cpp wrapper
│   │   └── coderunner.cpp         # C++ compile/run
│   ├── include/
│   │   ├── promptshell.h
│   │   ├── assistant_mode.h       # Polymorphic modes
│   │   ├── llm_engine.h
│   │   └── coderunner.h
│   └── CMakeLists.txt
├── models/
│   └── tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf  # AI model
├── temp_slides/
│   ├── 01-title.html
│   ├── 02-why-required.html
│   ├── 03-flowchart.html
│   ├── 04-cpp-oop.html
│   ├── 05-differentiator.html
│   └── 06-thanks.html
├── install.ps1                    # Windows installer
├── install.sh                     # Linux/macOS installer
├── CMakeLists.txt
├── README.md                      # This file
└── LICENSE
```

---

## 🔧 Architecture

### Core Components

| Component | Role |
|-----------|------|
| **PromptShell** | Main orchestrator; coordinates compile, run, debug workflows |
| **LLMEngine** | Wraps llama.cpp; executes inference with timeout |
| **CodeRunner** | Compiles C++ and captures compiler output |
| **AssistantMode** | Polymorphic behavior (Default, Debug, Concise, CodeOnly) |

### OOP Design Principles

- **Encapsulation**: Each class manages its own state and responsibilities
- **Abstraction**: Users see simple commands; internals stay hidden
- **Inheritance**: AssistantMode base class; specialized modes extend it
- **Polymorphism**: Modes can be swapped without changing main flow

### Technology Stack

| Technology | Purpose |
|-----------|---------|
| **C++17** | Backend language |
| **CMake + Ninja** | Build system |
| **llama.cpp** | Local LLM inference engine |
| **TinyLlama 1.1B** | Local AI model (300MB) |
| **MSYS2 MinGW64** (Windows) | Cross-platform compiler |
| **g++** (Linux/macOS) | Native compiler |

---

## 🎯 Features

✅ **Local AI** - No cloud dependencies, runs entirely offline  
✅ **CLI-First** - Designed for terminal workflow integration  
✅ **OOP Architecture** - Modular, extensible C++ codebase  
✅ **Multiple Modes** - Debug, run, shell, ask modes  
✅ **Fast Inference** - TinyLlama runs locally (~2-3s per response)  
✅ **Cross-Platform** - Windows, Linux, macOS support  
✅ **Resume-Ready** - Clean architecture suitable for portfolio showcasing  

---

## 🤝 Comparison with ChatGPT

| Feature | PromptShell | ChatGPT |
|---------|-------------|---------|
| **Internet Required** | ❌ No | ✅ Yes |
| **CLI Integration** | ✅ Yes | ❌ No (web-based) |
| **Code File Aware** | ✅ Yes | ⚠️ Requires copy-paste |
| **Compiler Output Integration** | ✅ Yes | ❌ Manual input |
| **Privacy** | ✅ Local data | ⚠️ Cloud-stored |
| **Speed** | ~2-3s | Network dependent |
| **AI Capability** | Good for code debugging | Advanced general knowledge |

---

## 🚀 Roadmap

### Phases 1-11 ✅ COMPLETE
- C++ backend with REPL
- llama.cpp integration
- OOP architecture (PromptShell, LLMEngine, CodeRunner, AssistantMode)
- Debug mode with AI explanations
- Typing animation
- Presentation deck
- Installation scripts

### Phases 12-15 🎯 IN PROGRESS
- **Phase 12**: VS Code extension UI
- **Phase 13**: Advanced prompting strategies
- **Phase 14**: Performance optimization
- **Phase 15**: Community features (templates, plugins)

---

## 🛠️ Troubleshooting

### Issue: `promptshell` command not found

**Solution (Windows):**
```powershell
# Restart PowerShell or reload profile
& $profile

# Or manually add to PATH
$BinDir = "C:\Users\YourName\PromptShell\build\bin"
[System.Environment]::SetEnvironmentVariable("PATH", "$env:PATH;$BinDir", [System.EnvironmentVariableTarget]::User)
```

**Solution (Linux/macOS):**
```bash
# Reload shell
source ~/.bashrc  # or ~/.zshrc

# Or manually add
export PATH="$PATH:$HOME/.promptshell/build/bin"
```

### Issue: Model not downloading

**Manual Download:**
1. Visit [HuggingFace](https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF)
2. Download: `tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf`
3. Save to: `PromptShell/models/`

### Issue: Build fails on Linux/macOS

**Ensure compiler is installed:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# macOS
xcode-select --install
```

### Issue: Slow inference

This is normal—TinyLlama is a 1.1B parameter model. Response time is typically 2-5 seconds depending on system hardware.

---

## 📚 Documentation

- **Architecture Guide**: See [backend/](./backend/) directory
- **Presentation Deck**: See [temp_slides/](./temp_slides/) (6 slides covering project overview)
- **Code Examples**: Check test files in backend/

---

## 🤝 Contributing

Contributions welcome! Areas to contribute:

- New assistant modes (performance analyzer, security checker, etc.)
- VS Code extension improvements
- Model optimization / smaller model support
- Bug reports and feature requests
- Documentation improvements

---

## 📄 License

MIT License - See [LICENSE](./LICENSE) file

---

## 👥 Authors

- **Moumita Baishya** (240102025)
- **Sahid Ahmed** (240102002)

---

## 💡 FAQ

**Q: Can I use larger AI models?**  
A: Yes, but you'll need more VRAM. See [llama.cpp documentation](https://github.com/ggerganov/llama.cpp).

**Q: How do I contribute a new assistant mode?**  
A: See [backend/include/assistant_mode.h](./backend/include/assistant_mode.h). New modes inherit from `AssistantMode`.

**Q: Does this work on Windows Subsystem for Linux (WSL)?**  
A: Yes! Use the Linux installation script inside WSL.

**Q: Can I use PromptShell for non-C++ languages?**  
A: Currently optimized for C++. Adding Python/JavaScript support is on the roadmap.

**Q: How much disk space does this need?**  
A: ~800MB (600MB model + 200MB build artifacts)

---

## 🔗 Links

- 🏠 [GitHub Repository](https://github.com/yourusername/PromptShell)
- 📖 [llama.cpp Project](https://github.com/ggerganov/llama.cpp)
- 🤗 [HuggingFace Models](https://huggingface.co/TheBloke)

---

**Made with ❤️ for local AI and clean code architecture**

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
