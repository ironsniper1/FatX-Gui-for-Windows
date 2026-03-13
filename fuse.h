#pragma once
// =============================================================================
// fuse_winfsp.h
// Drop-in replacement for <fuse.h> / <fuse/fuse.h> when building fusefatx
// against WinFsp on Windows.
//
// WinFsp ships its own FUSE-compatible header at:
//   C:\Program Files (x86)\WinFsp\inc\fuse\fuse.h   (FUSE 2.x API)
//   C:\Program Files (x86)\WinFsp\inc\fuse3\fuse.h  (FUSE 3.x API)
//
// This file simply redirects to WinFsp's header and patches a handful of
// differences so fatx.cpp compiles without changes.
// =============================================================================

#ifndef _WIN32
#  error "fuse_winfsp.h is for Windows/WinFsp builds only"
#endif

// Locate the WinFsp include directory.
// Override WINFSP_INC on the compiler command line if you installed WinFsp
// in a non-default location.
#ifndef WINFSP_INC
#  define WINFSP_INC "C:/Program Files (x86)/WinFsp/inc/fuse"
#endif

// WinFsp requires this before including its fuse header
#define FUSE_USE_VERSION 26

// Pull in WinFsp's FUSE 2 compatibility header
#include WINFSP_INC "/fuse.h"

// ── Patches for differences between Linux libfuse and WinFsp ─────────────────

// 1. fuse_main signature: WinFsp matches Linux FUSE 2 — no patch needed.

// 2. struct fuse_context: WinFsp provides this; no patch needed.

// 3. fuse_get_context(): provided by WinFsp; no patch needed.

// 4. struct stat on Windows lacks st_blocks / st_blksize.
//    fatx.cpp sets these; we just silently ignore them if missing.
#ifndef FUSE_STAT_HAS_BLOCKS
static inline void _fuse_set_blocks(struct stat* st, off_t size, int blksize) {
    (void)st; (void)size; (void)blksize;
    // no-op on Windows — WinFsp doesn't use st_blocks
}
// Macro used in fusefatx.cpp (if any) to set these fields:
#define FUSE_FILL_STAT_BLOCKS(st, sz, bsz) _fuse_set_blocks(st, sz, bsz)
#endif

// 5. Linux fuse.h defines fuse_fill_dir_t differently in older versions.
//    WinFsp FUSE 2 is compatible — no patch needed.

// 6. fuse_opt_parse: provided by WinFsp.

// ── End of shim ───────────────────────────────────────────────────────────────
