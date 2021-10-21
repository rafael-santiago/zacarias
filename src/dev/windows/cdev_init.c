/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/cdev_init.h>
#include <windows/cdev_create.h>
#include <windows/cdev_ioctl.h>
#include <windows/cdev_noimpl.h>
#include <defs/io.h>
#include <ctx/ctx.h>

NTSTATUS cdev_init(_In_ PDRIVER_OBJECT driver_object) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    cdev_mtx_init(&g_cdev()->lock);

    zacarias_profiles_ctx_init(g_cdev()->profiles);

    status = IoCreateDevice(driver_object, 0, &gZacariasDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_cdev()->device);

    if (!NT_SUCCESS(status)) {
        KdPrint(("/dev/zacarias error: Error while creating device.\n"));
        return status;
    }

    status = IoCreateSymbolicLink(&gZacariasSymLinkName, &gZacariasDeviceName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("/dev/zacarias error: Error while creating symbolic link.\n"));
        IoDeleteDevice(g_cdev()->device);
        g_cdev()->device = NULL;
        return status;
    }

    for (size_t f = 0; f < IRP_MJ_MAXIMUM_FUNCTION; f++) {
        driver_object->MajorFunction[f] = cdev_noimpl;
    }

    driver_object->MajorFunction[IRP_MJ_CREATE] = cdev_create;
    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = cdev_ioctl;
    
    return status;
}
