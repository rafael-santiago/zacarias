/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <linux/cdev_open.h>
#include <defs/types.h>
#include <linux/slab.h>

int cdev_open(struct inode *ip, struct file *fp) {
    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        printk(KERN_INFO "cdev_open: fail.\n");
        return EBUSY;
    }
    printk(KERN_INFO "cdev_open: ok.\n");
    cdev_mtx_unlock(&g_cdev()->lock);
    return 0;
}
