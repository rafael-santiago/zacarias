/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <freebsd/cdev_open.h>
#include <defs/types.h>

int cdev_open(struct cdev *dev __unused, int flags __unused, int devtype __unused, struct thread *td __unused) {
    if (cdev_mtx_trylock(&g_cdev()->lock)) {
        return EBUSY;
    }
    cdev_mtx_unlock(&g_cdev()->lock);
    return 0;
}
