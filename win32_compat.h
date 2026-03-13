#pragma once
// =============================================================================
// win32_compat.h
// Shim layer: replaces Linux/POSIX headers and types that fatx.cpp relies on.
// Include this as the FIRST header in fatx.cpp on Windows builds.
// =============================================================================

#ifndef _WIN32
#  error "This header is for Windows builds only"
#endif

// ── Standard Windows headers ─────────────────────────────────────────────────
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// ── POSIX types missing on MSVC ───────────────────────────────────────────────
#ifndef _SSIZE_T_DEFINED
typedef intptr_t ssize_t;
#define _SSIZE_T_DEFINED
#endif

#ifndef _MODE_T_DEFINED
typedef unsigned int mode_t;
#define _MODE_T_DEFINED
#endif

#ifndef _UID_T_DEFINED
typedef unsigned int uid_t;
typedef unsigned int gid_t;
#define _UID_T_DEFINED
#endif

#ifndef _PID_T_DEFINED
typedef int pid_t;
#define _PID_T_DEFINED
#endif

// ── POSIX file descriptor helpers ─────────────────────────────────────────────
#ifndef S_ISREG
#  define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISLNK
#  define S_ISLNK(m)  (0)   // no symlinks on Windows
#endif

// stat field aliases
#ifndef st_atim
#  define st_atim  st_atime
#  define st_mtim  st_mtime
#  define st_ctim  st_ctime
#endif

// ── Missing POSIX functions ───────────────────────────────────────────────────
#include <string.h>

static inline int strcasecmp(const char* a, const char* b) {
    return _stricmp(a, b);
}
static inline int strncasecmp(const char* a, const char* b, size_t n) {
    return _strnicmp(a, b, n);
}

// unistd.h equivalents
static inline int getuid() { return 0; }
static inline int getgid() { return 0; }

// ── File-path separator normalisation ─────────────────────────────────────────
// fatx.cpp uses '/' internally; Windows also accepts '/' in most file APIs so
// we just make sure the constant matches.
#ifndef sepdir
#  define sepdir  '/'
#endif

// ── Console / terminal ───────────────────────────────────────────────────────
// fatx uses isatty() to detect interactive mode
#ifndef _UNISTD_H
static inline int isatty(int fd) { return _isatty(fd); }
#endif

// ── Large-file support ────────────────────────────────────────────────────────
// MSVC uses _fseeki64 / _ftelli64; provide POSIX names as macros.
#define fseeko  _fseeki64
#define ftello  _ftelli64
typedef __int64  off_t;    // override if not already defined

// ── dirent emulation (needed only if fatx.cpp scans directories) ─────────────
// fatx.cpp does NOT use opendir/readdir directly, so we skip the full shim.

// ── Disable POSIX deprecation warnings from MSVC ─────────────────────────────
#pragma warning(disable: 4996)  // 'fopen': POSIX name deprecated

// ── Suppress common Boost + Windows macro conflicts ──────────────────────────
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif
