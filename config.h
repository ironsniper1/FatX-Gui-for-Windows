/* config.h — Windows stub generated for the FATX Windows port */
#pragma once

#define PACKAGE_NAME    "fatx"
#define PACKAGE_VERSION "1.18"
#define PACKAGE_STRING  "fatx 1.18"
#define PACKAGE_BUGREPORT ""
#define VERSION         "1.18"

/* fatx.cpp guards some Linux-only sections on this */
#define _WIN32_PORT 1

/* Indicate 64-bit file offset support */
#define _FILE_OFFSET_BITS 64

/* Boost is always present when building */
#define HAVE_BOOST 1
