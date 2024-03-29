/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_FREEBSD_CDEV_IOCTL_H
#define ZACARIAS_DEV_FREEBSD_CDEV_IOCTL_H 1

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

int cdev_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data, int flag __unused, struct thread *td);

#endif
