# Building FATX Natively on Windows
## Complete step-by-step guide

---

## What you need

| Tool | Where to get it | Notes |
|------|----------------|-------|
| Visual Studio 2022 (Community is free) | https://visualstudio.microsoft.com/ | Install "Desktop development with C++" workload |
| CMake 3.20+ | https://cmake.org/download/ | Tick "Add to PATH" during install |
| vcpkg | https://github.com/microsoft/vcpkg | Microsoft's C++ package manager |
| WinFsp | https://winfsp.dev/rel/ | Only needed for `fusefatx` |
| Git | https://git-scm.com/ | To clone repos |

---

## Step 1 — Get the FATX source code

Open a terminal (PowerShell or Command Prompt) and run:

```powershell
git clone https://github.com/baxter104/fatx.git
cd fatx
```

---

## Step 2 — Set up vcpkg (one-time)

```powershell
# Clone vcpkg next to your fatx folder (or anywhere you like)
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Install Boost (the subset fatx uses: format, bimap, multi_index, ptr_container)
.\vcpkg install boost:x64-windows
```

This takes 5–15 minutes the first time.

---

## Step 3 — Copy the Windows port files into the fatx source tree

Copy all files from this `fatx-win` folder into your cloned `fatx/` directory:

```
fatx/                        ← your git clone
├── fatx.cpp                 ← original source (DO NOT EDIT)
├── CMakeLists.txt           ← REPLACE with the one from fatx-win/
├── include/
│   └── win32/
│       ├── win32_compat.h   ← copy from fatx-win/include/win32/
│       └── config.h         ← copy from fatx-win/include/win32/
└── winfsp-shim/
    └── fuse.h               ← copy from fatx-win/winfsp-shim/
```

---

## Step 4 — Patch fatx.cpp (minimal changes needed)

Open `fatx.cpp` and add this block at the very top, before any other includes:

```cpp
// ── Windows port ─────────────────────────────────────────────────────────────
#ifdef _WIN32_PORT
#  include "win32/win32_compat.h"
#  ifdef FATX_FUSE_BUILD
#    include "fuse.h"   // resolves to winfsp-shim/fuse.h
#  endif
#else
// Original Linux includes follow
#  include "config.h"
#  include <unistd.h>
#  ifdef FATX_FUSE_BUILD
#    include <fuse.h>
#  endif
#endif
// ── End Windows port block ────────────────────────────────────────────────────
```

Also find anywhere fatx.cpp uses `argv[0]` to detect which tool to run, e.g.:

```cpp
string toolname = basename(argv[0]);
```

On Windows, `basename()` doesn't exist. Replace with:

```cpp
#ifdef _WIN32
    // Strip path and extension from argv[0]
    char fname[_MAX_FNAME];
    _splitpath_s(argv[0], nullptr, 0, nullptr, 0, fname, _MAX_FNAME, nullptr, 0);
    string toolname = fname;
#else
    string toolname = basename(argv[0]);
#endif
```

---

## Step 5 — Install WinFsp (for fusefatx only)

Download and run the installer from https://winfsp.dev/rel/

During installation, make sure to tick **"Developer"** to get the headers and .lib files.

Default install path: `C:\Program Files (x86)\WinFsp`

---

## Step 6 — Configure and build

```powershell
cd fatx   # your source directory

# Create a build folder
mkdir build
cd build

# Configure with CMake, pointing at your vcpkg toolchain
cmake .. `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release

# Build all tools
cmake --build . --config Release

# Binaries will appear in:  build\bin\Release\
#   mkfs.fatx.exe
#   fsck.fatx.exe
#   label.fatx.exe
#   unrm.fatx.exe
#   fusefatx.exe     (only if WinFsp was found)
```

---

## Step 7 — Test it

```powershell
cd build\bin\Release

# Create a test image
.\mkfs.fatx.exe -h

# Check a disk image
.\fsck.fatx.exe -t myimage.img

# View / set label
.\label.fatx.exe myimage.img
.\label.fatx.exe myimage.img -l MYVOL

# Recover deleted files
.\unrm.fatx.exe myimage.img -l C:\recovered\

# Mount as a drive letter (requires WinFsp installed on the machine)
.\fusefatx.exe myimage.img X:
```

---

## Common build errors and fixes

### `boost/format.hpp: No such file`
vcpkg Boost wasn't found. Make sure you passed `-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake` to cmake.

### `C2065: 'uid_t': undeclared identifier`
The `win32_compat.h` wasn't included first. Check that the `#ifdef _WIN32_PORT` block is at the very top of `fatx.cpp`.

### `LNK1104: cannot open winfsp-x64.lib`
WinFsp developer files weren't installed. Re-run the WinFsp installer and tick "Developer".

### `basename` not found
Apply the `_splitpath_s` patch from Step 4 above.

### Boost `advance` ambiguous (you may see this in older Boost versions)
This is a known Boost + MSVC issue. Add to CMakeLists.txt:
```cmake
add_compile_definitions(BOOST_MSVC_WAIT_FOR_ITERATOR_SPECIALIZATION)
```
Or upgrade to Boost 1.82+ via vcpkg (`.\vcpkg install boost:x64-windows` already gets latest).

---

## Runtime requirements (distributing the .exe)

When distributing the built executables to other machines:

- **mkfs / fsck / label / unrm**: only need the Visual C++ Redistributable
  (`vc_redist.x64.exe` — free from Microsoft)
- **fusefatx**: additionally requires WinFsp to be installed on the target machine
  (the WinFsp runtime, not the developer package)

You can bundle both in an installer with NSIS or Inno Setup.

---

## Architecture of the port

```
fatx.cpp  (original, minimal edits)
    │
    ├── #ifdef _WIN32_PORT
    │       win32_compat.h     ← replaces unistd.h, sys/stat.h, etc.
    │       config.h           ← replaces autoconf-generated config.h
    │
    ├── #ifdef FATX_FUSE_BUILD (fusefatx only)
    │       winfsp-shim/fuse.h → WinFsp's FUSE 2 header
    │                            maps fuse_main(), fuse_operations, etc.
    │                            to WinFsp's implementation
    │
    └── Boost (via vcpkg)
            boost::format, boost::ptr_vector,
            boost::bimap, boost::multi_index
```
