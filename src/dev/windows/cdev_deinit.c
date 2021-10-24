/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/cdev_deinit.h>
#include <defs/types.h>

void cdev_deinit(void) {
    IoDeleteSymbolicLink(&gZacariasSymLinkName);
    IoDeleteDevice(g_cdev()->device);

    zacarias_profiles_ctx_deinit(g_cdev()->profiles);

    KdPrint(("/dev/zacarias: Device Deinitialized.\n"));
}
