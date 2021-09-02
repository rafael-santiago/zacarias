/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <linux/cdev_init.h>
#include <linux/cdev_ioctl.h>
#include <linux/cdev_open.h>
#include <defs/types.h>
#include <defs/io.h>
#include <ctx/ctx.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/cdev.h>

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = cdev_open,
    .unlocked_ioctl = cdev_ioctl,
};

int zcdev_init(void) {
    int ret = -1;
    struct device *device = NULL;

    cdev_mtx_init(&g_cdev()->lock);

    zacarias_profiles_ctx_init(g_cdev()->profiles);

    if ((ret = alloc_chrdev_region(&g_cdev()->first, 0, 1, CDEVNAME)) < 0) {
        printk(KERN_INFO "/dev/zacarias: Error during cdev registration.\n");
        return ret;
    }

    g_cdev()->device_class = class_create(THIS_MODULE, CDEVCLASS);

    if (IS_ERR(g_cdev()->device_class)) {
        unregister_chrdev(g_cdev()->major_nr, CDEVNAME);
        printk(KERN_INFO "/dev/zacarias: Class creation has failed.\n");
        return PTR_ERR(g_cdev()->device_class);
    }

    device = device_create(g_cdev()->device_class, NULL, g_cdev()->first, NULL, CDEVNAME);

    if (IS_ERR(device)) {
        class_destroy(g_cdev()->device_class);
        unregister_chrdev(g_cdev()->major_nr, CDEVNAME);
        printk(KERN_INFO "/dev/zacarias: Device file creation failure.\n");
        return PTR_ERR(device);
    }

    cdev_init(&g_cdev()->c_dev, &fops);

    if ((ret = cdev_add(&g_cdev()->c_dev, g_cdev()->first, 1)) < 0) {
        device_destroy(g_cdev()->device_class, g_cdev()->first);
        class_destroy(g_cdev()->device_class);
        unregister_chrdev_region(g_cdev()->first, 1);
    }

    printk(KERN_INFO "/dev/zacarias: Device initialized.\n");

    return 0;
}
