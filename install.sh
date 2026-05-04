#!/bin/bash
# PromptShell Installation Script for Linux & macOS
# Usage: bash <(curl -sL https://raw.githubusercontent.com/yourusername/PromptShell/main/install.sh)
# Or: bash install.sh

set -e

SKIP_BUILD=false
SKIP_MODEL=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-build) SKIP_BUILD=true; shift ;;
        --skip-model) SKIP_MODEL=true; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
    INSTALL_DIR="${PROMPTSHELL_HOME:-$HOME/.promptshell}"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
    INSTALL_DIR="${PROMPTSHELL_HOME:-$HOME/Library/Application Support/PromptShell}"
else
    echo "❌ Unsupported OS: $OSTYPE"
    exit 1
fi

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║        PromptShell Installation for $OS$(printf '%*s' $((62 - ${#OS})) | tr ' ' ' ')║"
echo "╚════════════════════════════════════════════════════════════════╝"

echo "📁 Installation directory: $INSTALL_DIR"

# Create directory
mkdir -p "$INSTALL_DIR"

# Step 1: Check prerequisites
echo ""
echo "[1/5] Checking prerequisites..."

check_command() {
    if command -v $1 &> /dev/null; then
        echo "  ✓ $1 found"
        return 0
    else
        echo "  ✗ $1 NOT found"
        return 1
    fi
}

MISSING=()
check_command "git" || MISSING+=("git")
check_command "cmake" || MISSING+=("cmake")
check_command "ninja" || MISSING+=("ninja")
check_command "g++" || MISSING+=("g++")

if [ ${#MISSING[@]} -gt 0 ]; then
    echo ""
    echo "⚠️  Missing tools: ${MISSING[*]}"
    echo ""
    if [[ "$OS" == "Linux" ]]; then
        echo "Install on Ubuntu/Debian:"
        echo "  sudo apt-get update"
        echo "  sudo apt-get install -y git cmake ninja-build build-essential"
        echo ""
        echo "Install on Fedora/RHEL:"
        echo "  sudo dnf install git cmake ninja-build gcc-c++"
    else
        echo "Install on macOS:"
        echo "  brew install git cmake ninja"
        echo "  xcode-select --install  # For g++"
    fi
    exit 1
fi

# Step 2: Clone or update repository
echo ""
echo "[2/5] Setting up repository..."

if [ -d "$INSTALL_DIR/.git" ]; then
    echo "  📦 Repository already exists, pulling latest..."
    cd "$INSTALL_DIR"
    git pull origin main > /dev/null 2>&1
    echo "  ✓ Repository updated"
else
    echo "  📥 Cloning repository..."
    git clone https://github.com/yourusername/PromptShell.git "$INSTALL_DIR" > /dev/null 2>&1
    echo "  ✓ Repository cloned"
fi

cd "$INSTALL_DIR"

# Step 3: Build backend
echo ""
echo "[3/5] Setting up C++ backend..."

if [ "$SKIP_BUILD" = false ]; then
    echo "  🔨 Building backend with CMake + Ninja..."
    rm -rf build 2>/dev/null || true
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1
    ninja -C build > /dev/null 2>&1
    
    if [ -f "build/bin/promptshell" ]; then
        echo "  ✓ Backend built successfully"
    else
        echo "  ✗ Build failed. Check errors above."
        exit 1
    fi
else
    echo "  ⏭️  Skipping build (--skip-build)"
fi

# Step 4: Download TinyLlama model
echo ""
echo "[4/5] Setting up AI model..."

mkdir -p models
MODEL_FILE="models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"
MODEL_URL="https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"

if [ -f "$MODEL_FILE" ]; then
    echo "  ✓ Model already downloaded"
elif [ "$SKIP_MODEL" = false ]; then
    echo "  📥 Downloading TinyLlama model (300MB)..."
    echo "     This may take a few minutes..."
    
    if command -v wget &> /dev/null; then
        wget -q --show-progress "$MODEL_URL" -O "$MODEL_FILE" 2>&1 || {
            echo "  ⚠️  Failed to download model. Download manually:"
            echo "     $MODEL_URL"
        }
    elif command -v curl &> /dev/null; then
        curl -L -o "$MODEL_FILE" "$MODEL_URL" 2>&1 || {
            echo "  ⚠️  Failed to download model. Download manually:"
            echo "     $MODEL_URL"
        }
    fi
    echo "  ✓ Model downloaded"
fi

# Step 5: Setup PATH
echo ""
echo "[5/5] Setting up global command..."

BIN_DIR="$INSTALL_DIR/build/bin"
SHELL_RC=""

if [ -f ~/.zshrc ]; then
    SHELL_RC="$HOME/.zshrc"
elif [ -f ~/.bashrc ]; then
    SHELL_RC="$HOME/.bashrc"
elif [ -f ~/.bash_profile ]; then
    SHELL_RC="$HOME/.bash_profile"
fi

if [ -n "$SHELL_RC" ]; then
    if ! grep -q "PROMPTSHELL" "$SHELL_RC"; then
        echo "" >> "$SHELL_RC"
        echo "# PromptShell" >> "$SHELL_RC"
        echo "export PROMPTSHELL_HOME=\"$INSTALL_DIR\"" >> "$SHELL_RC"
        echo "export PATH=\"\$PATH:$BIN_DIR\"" >> "$SHELL_RC"
        echo "  ✓ Added to $SHELL_RC"
    else
        echo "  ✓ Already in $SHELL_RC"
    fi
else
    echo "  ⚠️  Could not detect shell RC file"
fi

# Make binary executable
chmod +x "$BIN_DIR/promptshell" 2>/dev/null || true

# Final summary
echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                  Installation Complete! 🎉                     ║"
echo "╚════════════════════════════════════════════════════════════════╝"

echo ""
echo "📝 Next Steps:"
echo "  1. Reload your shell:"
if [ -n "$SHELL_RC" ]; then
    echo "     source $SHELL_RC"
else
    echo "     source ~/.bashrc  (or ~/.zshrc)"
fi
echo "  2. Test the installation:"
echo "     promptshell --help"
echo "  3. Create a test C++ file and run:"
echo "     promptshell debug test.cpp"

echo ""
echo "📚 Documentation:"
echo "  • README: $INSTALL_DIR/README.md"
echo "  • Homepage: https://github.com/yourusername/PromptShell"
