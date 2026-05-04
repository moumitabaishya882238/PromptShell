# Development Environment Setup Guide

## Status Check

### Current System
- **OS**: Windows 10/11
- **C++ Compiler**: ✅ G++ (MSYS2 v15.1.0)
- **CMake**: ❌ NOT INSTALLED (Required)
- **Git**: Verify below

## Step 1: Install CMake (REQUIRED)

### Option A: Using Chocolatey (Recommended)
```powershell
choco install cmake
```

### Option B: Using Windows Package Manager
```powershell
winget install Kitware.CMake
```

### Option C: Manual Download
1. Visit: https://cmake.org/download/
2. Download: Windows x64 ZIP or MSI
3. Add to PATH

### Verify Installation
```powershell
cmake --version
```

---

## Step 2: Verify Git Installation
```powershell
git --version
```

If not installed, download from: https://git-scm.com/download/win

---

## Step 3: Verify All Tools
```powershell
g++ --version
cmake --version
git --version
```

All three should show versions.

---

## Next Steps After Setup

Once all tools are installed:
1. Clone llama.cpp
2. Build llama.cpp
3. Download a GGUF model

See `docs/phase2-setup.md` for detailed instructions.
