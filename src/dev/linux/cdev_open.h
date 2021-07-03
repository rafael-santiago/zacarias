/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_LINUX_CDEV_OPEN_H
#define ZACARIAS_DEV_LINUX_CDEV_OPEN_H 1

#include <linux/fs.h>

int cdev_open(struct inode *ip, struct file *fp);

#endif
