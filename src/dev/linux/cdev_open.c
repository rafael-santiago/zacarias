/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <defs/types.h>
#include <defs/zc_dbg.h>
#include <linux/cdev_open.h>
#include <linux/slab.h>

int cdev_open(struct inode *ip, struct file *fp) {
    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }
    cdev_mtx_unlock(&g_cdev()->lock);
    return 0;
}
