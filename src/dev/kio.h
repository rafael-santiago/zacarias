#ifndef ZACARIAS_DEV_KIO_H
#define ZACARIAS_DEV_KIO_H 1

#if defined(__linux__)
# include <linux/kio_impl.h>
#elif defined(__FreeBSD__)
# include <freebsd/kio_impl.h>
#elif defined(__NetBSD__)
# include <netbsd/kio_impl.h>
#else
# error Some code wanted.
#endif

#define kwrite(f, b, bs) kwrite_impl((f), (void *)(b), (bs))
#define kread(f, b, bs) kread_impl((f), (void *)(b), (bs))
#define kread_pwdb(f, p, ps, b, bs) kread_pwdb_impl((f), (p), (ps), (void *)(b), (bs))

#endif
