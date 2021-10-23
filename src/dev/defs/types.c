/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <defs/types.h>

static struct cdev_ctx g_cdev_data;

struct cdev_ctx *g_cdev(void) {
    return &g_cdev_data;
}

#if defined(_WIN32)
UNICODE_STRING gZacariasDeviceName = RTL_CONSTANT_STRING(L"\\Device\\ZacariasDevice");
UNICODE_STRING gZacariasSymLinkName = RTL_CONSTANT_STRING(L"\\??\\ZacariasDeviceLink");

int cdev_mtx_trylock(KMUTEX *m) {
    LARGE_INTEGER tmo = { 0 };
    return NT_SUCCESS(KeWaitForSingleObject(m, Executive, KernelMode, FALSE, &tmo));
}

#endif // defined(_WIN32)
