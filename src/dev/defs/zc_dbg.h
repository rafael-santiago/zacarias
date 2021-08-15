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
#  define ZC_DBG(message, ...) uprintf("[%s:%d] " message, __FILE__, __LINE__, ## __VA_ARGS__)
# else
#  define ZC_DBG(message, ...)
# endif
#endif

#endif
