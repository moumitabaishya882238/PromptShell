# PromptShell Installation Script for Windows
# Usage: powershell -ExecutionPolicy Bypass -File install.ps1

param(
    [switch]$SkipBuild = $false,
    [switch]$SkipModel = $false
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║           PromptShell Installation for Windows                 ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

# Detect installation directory
$InstallDir = if ($env:PROMPTSHELL_HOME) { $env:PROMPTSHELL_HOME } else { "$env:USERPROFILE\PromptShell" }
Write-Host "📁 Installation directory: $InstallDir" -ForegroundColor Green

# Ensure directory exists
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Write-Host "✓ Created directory" -ForegroundColor Green
}

# Step 1: Check prerequisites
Write-Host "`n[1/5] Checking prerequisites..." -ForegroundColor Yellow

$Prerequisites = @{
    "git"   = "git --version"
    "cmake" = "cmake --version"
    "ninja" = "ninja --version"
}

$MissingTools = @()
foreach ($tool in $Prerequisites.Keys) {
    try {
        & cmd /c "$($Prerequisites[$tool])" 2>&1 | Out-Null
        Write-Host "  ✓ $tool found" -ForegroundColor Green
    }
    catch {
        $MissingTools += $tool
        Write-Host "  ✗ $tool NOT found" -ForegroundColor Red
    }
}

if ($MissingTools.Count -gt 0) {
    Write-Host "`n⚠️  Missing tools: $($MissingTools -join ', ')" -ForegroundColor Yellow
    Write-Host "`nPlease install them:" -ForegroundColor Yellow
    Write-Host "  • Git: https://git-scm.com/download/win" -ForegroundColor Cyan
    Write-Host "  • CMake: https://cmake.org/download/" -ForegroundColor Cyan
    Write-Host "  • Ninja: https://github.com/ninja-build/ninja/releases" -ForegroundColor Cyan
    Write-Host "  • MinGW64 (MSYS2): https://www.msys2.org/" -ForegroundColor Cyan
    exit 1
}

# Step 2: Clone or update repository
Write-Host "`n[2/5] Setting up repository..." -ForegroundColor Yellow

if (Test-Path "$InstallDir\.git") {
    Write-Host "  📦 Repository already exists, pulling latest..." -ForegroundColor Cyan
    Push-Location $InstallDir
    & git pull origin main 2>&1 | Out-Null
    Pop-Location
    Write-Host "  ✓ Repository updated" -ForegroundColor Green
}
else {
    Write-Host "  📥 Cloning repository..." -ForegroundColor Cyan
    & git clone https://github.com/yourusername/PromptShell.git $InstallDir 2>&1 | Out-Null
    Write-Host "  ✓ Repository cloned" -ForegroundColor Green
}

# Step 3: Download or build backend
Write-Host "`n[3/5] Setting up C++ backend..." -ForegroundColor Yellow

Push-Location $InstallDir

if (-not $SkipBuild) {
    Write-Host "  🔨 Building backend with CMake + Ninja..." -ForegroundColor Cyan
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build" | Out-Null
    }
    
    & cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    & ninja -C build 2>&1 | Out-Null
    
    if (Test-Path "build\bin\promptshell.exe") {
        Write-Host "  ✓ Backend built successfully" -ForegroundColor Green
    }
    else {
        Write-Host "  ✗ Build failed. Check build errors above." -ForegroundColor Red
        Pop-Location
        exit 1
    }
}
else {
    Write-Host "  ⏭️  Skipping build (--SkipBuild)" -ForegroundColor Cyan
}

# Step 4: Download TinyLlama model
Write-Host "`n[4/5] Setting up AI model..." -ForegroundColor Yellow

$ModelsDir = "$InstallDir\models"
if (-not (Test-Path $ModelsDir)) {
    New-Item -ItemType Directory -Path $ModelsDir -Force | Out-Null
}

$ModelFile = "$ModelsDir\tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"
$ModelUrl = "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"

if (Test-Path $ModelFile) {
    Write-Host "  ✓ Model already downloaded" -ForegroundColor Green
}
elseif (-not $SkipModel) {
    Write-Host "  📥 Downloading TinyLlama model (300MB)..." -ForegroundColor Cyan
    Write-Host "     This may take a few minutes..." -ForegroundColor Cyan
    
    try {
        $WebClient = New-Object System.Net.WebClient
        $WebClient.DownloadFile($ModelUrl, $ModelFile)
        Write-Host "  ✓ Model downloaded" -ForegroundColor Green
    }
    catch {
        Write-Host "  ⚠️  Failed to download model. You can download it manually:" -ForegroundColor Yellow
        Write-Host "     $ModelUrl" -ForegroundColor Cyan
    }
}

# Step 5: Setup PATH and command alias
Write-Host "`n[5/5] Setting up global command..." -ForegroundColor Yellow

$BinDir = "$InstallDir\build\bin"
$CmdFile = "$BinDir\promptshell.cmd"

# Create wrapper command
@"
@echo off
"$BinDir\promptshell.exe" %*
"@ | Out-File -Encoding ASCII -FilePath $CmdFile -Force

Write-Host "  ✓ Created wrapper command at $CmdFile" -ForegroundColor Green

# Add to PATH if not already present
$UserPath = [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::User)
if (-not $UserPath.Contains($BinDir)) {
    [System.Environment]::SetEnvironmentVariable("PATH", "$UserPath;$BinDir", [System.EnvironmentVariableTarget]::User)
    Write-Host "  ✓ Added to USER PATH" -ForegroundColor Green
    Write-Host "  💡 Restart your terminal for PATH changes to take effect" -ForegroundColor Yellow
}
else {
    Write-Host "  ✓ Already in PATH" -ForegroundColor Green
}

Pop-Location

# Final summary
Write-Host "`n╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                  Installation Complete! 🎉                     ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

Write-Host "`n📝 Next Steps:" -ForegroundColor Green
Write-Host "  1. Restart your terminal or run: `$profile` to reload PATH" -ForegroundColor Cyan
Write-Host "  2. Test the installation:" -ForegroundColor Cyan
Write-Host "     promptshell --help" -ForegroundColor Cyan
Write-Host "  3. Create a test C++ file and run:" -ForegroundColor Cyan
Write-Host "     promptshell debug test.cpp" -ForegroundColor Cyan

Write-Host "`n📚 Documentation:" -ForegroundColor Green
Write-Host "  • README: $InstallDir\README.md" -ForegroundColor Cyan
Write-Host "  • Homepage: https://github.com/yourusername/PromptShell" -ForegroundColor Cyan
