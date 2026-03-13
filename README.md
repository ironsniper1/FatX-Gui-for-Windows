# FATX Tools for Windows

A native Windows port of the [FATX filesystem tools](https://github.com/baxter104/fatx), originally written for Linux. Includes a native Win32 GUI for easy use without the command line.

FATX is the filesystem used by the original Xbox and Xbox 360. These tools let you check, repair, format, and recover files from FATX disk images on Windows.

---

## Tools

| Executable | Description |
|---|---|
| `fatx_gui.exe` | Native Windows GUI — wraps all four tools with a tabbed interface |
| `fsck.fatx.exe` | Check and repair a FATX filesystem |
| `mkfs.fatx.exe` | Format a disk image with a new FATX filesystem |
| `unrm.fatx.exe` | Recover deleted files from a FATX filesystem |
| `label.fatx.exe` | Display or change the volume label |

---

## Download

Grab the latest release from the [Releases](../../releases) page. All executables are statically linked — no install, no Visual C++ redistributables needed. Just unzip and run.

---

## Usage

### GUI

Launch `fatx_gui.exe`. Select a tab for the tool you want, browse to your disk image, configure options, and click **Run**. Output streams live into the console area at the bottom.

### Command Line

All tools follow the same basic pattern:

```
fsck.fatx.exe [options] <image>
mkfs.fatx.exe [options] <image>
unrm.fatx.exe [options] <image>
label.fatx.exe [options] <image> [new-label]
```

Common options:

| Flag | Description |
|---|---|
| `-t <type>` | Table type: `hd`, `file`, `mu`, `usb`, `kit` |
| `-p <part>` | Partition: `x2`, `x1`, `sc`, `gc`, `se1`, `se2`, `xdv` |
| `-v` | Verbose output |
| `-w` | Write changes (required to actually repair/modify) |
| `-y` | Auto-answer yes to all prompts |

Run any tool with no arguments to see its full usage.

---

## Building from Source

### Requirements

- Visual Studio 2022 or 2026 with C++ workload
- [CMake](https://cmake.org/) 3.20+
- [vcpkg](https://github.com/microsoft/vcpkg)
- Boost (installed via vcpkg)

### Steps

**1. Install vcpkg and Boost:**
```powershell
cd C:\vcpkg
.\vcpkg install boost:x64-windows-static
```

**2. Clone and build:**
```powershell
git clone https://github.com/yourusername/fatx-windows
cd fatx-windows
```
Open the folder in Visual Studio, select the `windows-x64-release` preset, and click **Build → Build All**.

Output executables will be in `out\build\windows-x64\bin\Release\`.

### Optional: WinFsp (FUSE mount support)

Install [WinFsp](https://winfsp.dev/rel/) with the **Developer** feature enabled, then rebuild. This adds `fusefatx.exe` which lets you mount FATX images as a drive letter in Windows Explorer.

---

## What's Different from the Linux Version

The original source targets Linux only (FUSE, POSIX types, GCC extensions). This port adds:

- `win32/win32_compat.h` — POSIX type stubs, errno values, and missing POSIX functions for MSVC
- `win32/winfsp_fuse.h` — FUSE3 → WinFsp bridge for optional mount support
- Patched source files for MSVC compatibility (no VLAs, no `%lu` for `size_t`, etc.)
- Native Win32 GUI (`fatx_gui.cpp`) with live output streaming
- CMake build system replacing the original Linux Makefile

---

## License

See [LICENSE](LICENSE). Original FATX tools by [baxter104](https://github.com/baxter104/fatx).
