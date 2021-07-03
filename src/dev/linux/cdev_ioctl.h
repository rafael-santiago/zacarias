/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_LINUX_CDEV_IOCTL_H
#define ZACARIAS_DEV_LINUX_CDEV_IOCTL_H 1

#include <linux/fs.h>

long cdev_ioctl(struct file *fp, unsigned int cmd, unsigned long user_param);

#endif
