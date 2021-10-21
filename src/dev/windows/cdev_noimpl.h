/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_WINDOWS_CDEV_NOIMPL_H
#define ZACARIAS_DEV_WINDOWS_CDEV_NOIMPL_H 1

#include <defs/types.h>

NTSTATUS cdev_noimpl(PDEVICE_OBJECT dev, PIRP irp);

#endif
