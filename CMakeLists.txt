cmake_minimum_required(VERSION 3.20)
project(fatx VERSION 1.18 LANGUAGES CXX)

# ── C++ standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── vcpkg toolchain (set via -DCMAKE_TOOLCHAIN_FILE on command line) ──────────
# Boost is pulled in via vcpkg; if you have a system Boost, adjust accordingly.

# ── Boost (header-only subset used by fatx: format, bimap, multi_index, ptr_vector)
find_package(Boost 1.74 REQUIRED)

# ── Source file ───────────────────────────────────────────────────────────────
# The entire FATX project compiles from a single fatx.cpp.
# The tool (mkfs / fsck / label / unrm / fusefatx) is selected at runtime via
# the executable name (argv[0]).  On Windows we build separate .exe files,
# each compiled with a #define that sets the default tool name.

set(FATX_SRC ${CMAKE_SOURCE_DIR}/fatx.cpp)

# ── Common compile definitions ────────────────────────────────────────────────
set(COMMON_DEFS
    _WIN32_PORT          # activates win32_compat.h inclusion inside fatx.cpp
    HAVE_CONFIG_H        # fatx.cpp checks this; we supply a stub config.h
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# ── Common include directories ────────────────────────────────────────────────
set(COMMON_INCS
    ${CMAKE_SOURCE_DIR}/include/win32   # win32_compat.h lives here
    ${Boost_INCLUDE_DIRS}
)

# ── MSVC-specific flags ───────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W3 /wd4996 /wd4244 /wd4267 /EHsc /bigobj)
    # /wd4996  – POSIX name deprecated
    # /wd4244  – conversion warnings (Boost bimap)
    # /wd4267  – size_t narrowing (Boost)
    # /bigobj  – fatx.cpp is large
endif()

# =============================================================================
# Helper macro: add one CLI tool exe
# =============================================================================
macro(add_fatx_tool TARGET_NAME TOOL_DEFINE)
    add_executable(${TARGET_NAME} ${FATX_SRC})
    target_compile_definitions(${TARGET_NAME} PRIVATE
        ${COMMON_DEFS}
        FATX_TOOL_NAME="${TOOL_DEFINE}"
    )
    target_include_directories(${TARGET_NAME} PRIVATE ${COMMON_INCS})
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${TARGET_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endmacro()

# =============================================================================
# Non-FUSE tools  (mkfs, fsck, label, unrm)
# These have NO libfuse dependency and compile cleanly on Windows.
# =============================================================================
add_fatx_tool(mkfs.fatx   "mkfs.fatx")
add_fatx_tool(fsck.fatx   "fsck.fatx")
add_fatx_tool(label.fatx  "label.fatx")
add_fatx_tool(unrm.fatx   "unrm.fatx")

# =============================================================================
# fusefatx  — requires WinFsp
# Only built if WinFsp is found; skip gracefully if not installed.
# =============================================================================
set(WINFSP_ROOT "C:/Program Files (x86)/WinFsp" CACHE PATH
    "Root of the WinFsp installation")

if(EXISTS "${WINFSP_ROOT}/inc/fuse/fuse.h")
    message(STATUS "WinFsp found at ${WINFSP_ROOT} — building fusefatx")

    add_executable(fusefatx ${FATX_SRC})
    target_compile_definitions(fusefatx PRIVATE
        ${COMMON_DEFS}
        FATX_TOOL_NAME="fusefatx"
        FATX_FUSE_BUILD           # tells fatx.cpp to include FUSE sections
        FUSE_USE_VERSION=26
    )
    target_include_directories(fusefatx PRIVATE
        ${COMMON_INCS}
        ${CMAKE_SOURCE_DIR}/winfsp-shim   # our fuse.h redirect
        "${WINFSP_ROOT}/inc/fuse"         # WinFsp FUSE headers
    )

    # Link against WinFsp DLL import lib
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WINFSP_LIB "${WINFSP_ROOT}/lib/winfsp-x64.lib")
    else()
        set(WINFSP_LIB "${WINFSP_ROOT}/lib/winfsp-x86.lib")
    endif()

    if(EXISTS "${WINFSP_LIB}")
        target_link_libraries(fusefatx PRIVATE ${WINFSP_LIB})
    else()
        message(WARNING "WinFsp import lib not found at ${WINFSP_LIB}")
    endif()

    set_target_properties(fusefatx PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
else()
    message(STATUS
        "WinFsp not found at ${WINFSP_ROOT} — fusefatx will NOT be built.\n"
        "Install WinFsp from https://winfsp.dev/rel/ and re-run cmake.")
endif()

# =============================================================================
# Install rules  (cmake --install . --prefix C:/fatx)
# =============================================================================
install(TARGETS mkfs.fatx fsck.fatx label.fatx unrm.fatx
        RUNTIME DESTINATION bin)
if(TARGET fusefatx)
    install(TARGETS fusefatx RUNTIME DESTINATION bin)
endif()
