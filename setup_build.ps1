# setup_build.ps1
# ============================================================
# One-shot script: installs prerequisites and builds FATX for
# Windows natively.  Run from the root of the fatx source tree
# with the fatx-win port files already copied in.
#
# Usage (PowerShell, run as Administrator):
#   Set-ExecutionPolicy Bypass -Scope Process
#   .\setup_build.ps1
# ============================================================

$ErrorActionPreference = "Stop"

# ── Config ────────────────────────────────────────────────────────────────────
$VCPKG_DIR   = "C:\vcpkg"
$WINFSP_URL  = "https://github.com/winfsp/winfsp/releases/download/v2.0/winfsp-2.0.23075.msi"
$WINFSP_MSI  = "$env:TEMP\winfsp.msi"
$BUILD_DIR   = ".\build"

# ── Helper ────────────────────────────────────────────────────────────────────
function Step($msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }
function OK($msg)   { Write-Host "    OK: $msg" -ForegroundColor Green }
function Warn($msg) { Write-Host "    WARN: $msg" -ForegroundColor Yellow }

# ── 1. Check for cmake ────────────────────────────────────────────────────────
Step "Checking CMake"
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Please install from https://cmake.org/download/" -ForegroundColor Red
    exit 1
}
OK "CMake $(cmake --version | Select-String '\d+\.\d+\.\d+' | ForEach-Object { $_.Matches[0].Value })"

# ── 2. Set up vcpkg ───────────────────────────────────────────────────────────
Step "Setting up vcpkg at $VCPKG_DIR"
if (-not (Test-Path "$VCPKG_DIR\vcpkg.exe")) {
    if (-not (Test-Path $VCPKG_DIR)) {
        git clone https://github.com/microsoft/vcpkg.git $VCPKG_DIR
    }
    Push-Location $VCPKG_DIR
    .\bootstrap-vcpkg.bat -disableMetrics
    Pop-Location
    OK "vcpkg bootstrapped"
} else {
    OK "vcpkg already installed"
}

# ── 3. Install Boost via vcpkg ────────────────────────────────────────────────
Step "Installing Boost (x64-windows) via vcpkg — this may take 10-15 min first time"
& "$VCPKG_DIR\vcpkg.exe" install boost:x64-windows
OK "Boost installed"

# ── 4. Download and install WinFsp (for fusefatx) ────────────────────────────
Step "Installing WinFsp (needed for fusefatx mount tool)"
$winfspInstalled = Test-Path "C:\Program Files (x86)\WinFsp\inc\fuse\fuse.h"
if (-not $winfspInstalled) {
    Write-Host "    Downloading WinFsp..."
    Invoke-WebRequest -Uri $WINFSP_URL -OutFile $WINFSP_MSI
    Write-Host "    Installing WinFsp (with Developer feature)..."
    Start-Process msiexec.exe -Wait -ArgumentList `
        "/i `"$WINFSP_MSI`" ADDLOCAL=Developer /quiet /norestart"
    OK "WinFsp installed"
} else {
    OK "WinFsp already installed"
}

# ── 5. CMake configure ────────────────────────────────────────────────────────
Step "Configuring CMake build"
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null

cmake . `
    -B $BUILD_DIR `
    -G "Visual Studio 17 2022" -A x64 `
    "-DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR\scripts\buildsystems\vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET=x64-windows `
    -DCMAKE_BUILD_TYPE=Release

OK "CMake configured"

# ── 6. Build ──────────────────────────────────────────────────────────────────
Step "Building (Release)"
cmake --build $BUILD_DIR --config Release --parallel
OK "Build complete"

# ── 7. Show results ───────────────────────────────────────────────────────────
Step "Results"
$binDir = "$BUILD_DIR\bin\Release"
if (Test-Path $binDir) {
    Get-ChildItem "$binDir\*.exe" | ForEach-Object {
        Write-Host "    $($_.Name)" -ForegroundColor White
    }
    Write-Host "`nExecutables are in: $binDir" -ForegroundColor Green
} else {
    Warn "Could not find $binDir — check build output above for errors"
}

Write-Host @"

Done! Quick test:
  cd $binDir
  .\mkfs.fatx.exe -h
  .\fsck.fatx.exe -h
  .\label.fatx.exe -h
  .\unrm.fatx.exe -h
  .\fusefatx.exe -h   (if WinFsp was found)
"@ -ForegroundColor Cyan
