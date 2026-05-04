param(
    [string]$Source = "E:\PromptShell\backend\build\bin\promptshell.exe",
    [string]$InstallDir = "$HOME\bin"
)

$ErrorActionPreference = 'Stop'

if (!(Test-Path $Source)) {
    throw "Source executable not found: $Source"
}

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
Copy-Item -Force $Source (Join-Path $InstallDir "promptshell.exe")

$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
if (-not $userPath) { $userPath = '' }

$parts = $userPath -split ';' | Where-Object { $_ -and $_.Trim() -ne '' }
if ($parts -notcontains $InstallDir) {
    $newPath = ($parts + $InstallDir) -join ';'
    [Environment]::SetEnvironmentVariable('Path', $newPath, 'User')
    Write-Host "Added $InstallDir to User PATH. Reopen terminals to use promptshell anywhere."
} else {
    Write-Host "$InstallDir is already on User PATH."
}

Write-Host "Installed promptshell.exe to $InstallDir"
