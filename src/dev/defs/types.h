/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_DEFS_TYPES_H
#define ZACARIAS_DEV_DEFS_TYPES_H 1

# if defined(__linux__)
#  include <linux/cdev.h>
#  include <linux/mutex.h>
   typedef struct mutex cdev_mtx;
#  define cdev_mtx_init(m) mutex_init((m))
#  define cdev_mtx_trylock(m) mutex_trylock((m))
#  define cdev_mtx_unlock(m) mutex_unlock((m))
#  define cdev_mtx_deinit(m) mutex_destroy((m))
# elif defined(__FreeBSD__)
#  include <sys/param.h>
#  include <sys/lock.h>
#  include <sys/mutex.h>
   typedef struct mtx cdev_mtx;
#  define cdev_mtx_init(m) mtx_init((m), "ZACARIAS_CDEV", NULL, MTX_DEF)
#  define cdev_mtx_trylock(m) mtx_trylock((m))
#  define cdev_mtx_unlock(m) mtx_unlock((m))
#  define cdev_mtx_deinit(m) mtx_destroy((m))
# elif defined(__NetBSD__)
#  include <sys/mutex.h>
   typedef kmutex_t cdev_mtx;
#  define cdev_mtx_init(m) mutex_init((m), MUTEX_DEFAULT, IPL_NONE)
#  define cdev_mtx_trylock(m) mutex_tryenter((m))
#  define cdev_mtx_unlock(m) mutex_exit((m))
#  define cdev_mtx_deinit(m) mutex_destroy((m))
# elif defined(_WIN32)
#  include <wdm.h>
   typedef KMUTEX cdev_mtx;
#  define cdev_mtx_init(m) KeInitializeMutex(m, 0)
#  define cdev_mtx_trylock(m) ( PLARGE_INTEGER __loctm = { 0 }, NT_SUCCESS(KeWaitForSingleObject(m, Executive, KernelMode, FALSE, &__loctm)) )
#  define cdev_mtx_unlock(m) KeReleaseMutex(m, FALSE)
#  define cdev_mtx_deinit(m) // ...
   extern UNICODE_STRING gZacariasDeviceName;
   extern UNICODE_STRING gZacariasSymLinkName;
# endif

#include <ctx/ctx.h>

struct cdev_ctx {
# if defined(__linux__)
    int major_nr;
    struct class *device_class;
    dev_t first;
    struct cdev c_dev;
# elif defined(__FreeBSD__)
    struct cdev *device;
# elif defined(_WIN32)
    PDEVICE_OBJECT device;
# endif
    cdev_mtx lock;
    zacarias_profiles_ctx *profiles;
};

struct cdev_ctx *g_cdev(void);

#define CDEVCLASS "zcs"

#endif
