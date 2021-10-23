/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_KIO_H
#define ZACARIAS_DEV_KIO_H 1

#if defined(__linux__)
# include <linux/kio_impl.h>
#elif defined(__FreeBSD__)
# include <freebsd/kio_impl.h>
#elif defined(__NetBSD__)
# include <netbsd/kio_impl.h>
#elif defined(_WIN32)
# include <windows/kio_impl.h>
#else
# error Some code wanted.
#endif

#define kwrite(f, b, bs) kwrite_impl((f), (void *)(b), (bs))
#define kread(f, b, bs) kread_impl((f), (void *)(b), (bs))
#define kread_pwdb(f, p, ps, b, bs) kread_pwdb_impl((f), (p), (ps), (void *)(b), (bs))

#endif
