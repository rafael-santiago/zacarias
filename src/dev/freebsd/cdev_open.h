/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_FREEBSD_CDEV_OPEN_H
#define ZACARIAS_DEV_FREEBSD_CDEV_OPEN_H 1

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

int cdev_open(struct cdev *dev __unused, int flags __unused, int devtype __unused, struct thread *td __unused);

#endif
