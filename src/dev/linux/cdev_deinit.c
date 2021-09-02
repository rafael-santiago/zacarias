/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <linux/cdev_deinit.h>
#include <defs/types.h>
#include <defs/io.h>
#include <ctx/ctx.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/cdev.h>

void zcdev_deinit(void) {
    cdev_del(&g_cdev()->c_dev);
    device_destroy(g_cdev()->device_class, g_cdev()->first);
    class_destroy(g_cdev()->device_class);
    unregister_chrdev_region(g_cdev()->first, 1);

    zacarias_profiles_ctx_deinit(g_cdev()->profiles);
    printk(KERN_INFO "/dev/zacarias: Device deinitialized.\n");
}
