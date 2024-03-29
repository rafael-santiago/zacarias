/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_DEFS_ZC_DBG_H
#define ZACARIAS_DEV_DEFS_ZC_DBG_H 1

// INFO(Rafael): Define the following macro if you want to log some debug info into device driver.
#undef ZACARIAS_DEBUG_INFO

#if defined(__linux__)
# if defined(ZACARIAS_DEBUG_INFO)
#  define ZC_DBG(message, ...) printk(KERN_INFO "[%s:%d] " message, __FILE__, __LINE__, ## __VA_ARGS__)
# else
#  define ZC_DBG(message, ...)
# endif
#elif defined(__FreeBSD__)
# if defined(ZACARIAS_DEBUG_INFO)
#  define ZC_DBG(message, ...) log(-1, "[%s:%d] " message, __FILE__, __LINE__, ## __VA_ARGS__)
# else
#  define ZC_DBG(message, ...)
# endif
#elif defined(_WIN32)
# if defined(ZACARIAS_DEBUG_INFO) && !defined(DBG)
#  pragma message("*** WARNING: You have defined ZACARIAS_DEBUG_INFO but you must compile the driver in debug configuration! [use --compile-model=debug]")
# endif
# if defined(ZACARIAS_DEBUG_INFO)
#  define ZC_DBG(message, ...) KdPrint(("[%s:%d] " message, __FILE__, __LINE__, ## __VA_ARGS__))
# else
#  define ZC_DBG(message, ...)
# endif
#endif

#endif
