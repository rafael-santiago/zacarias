/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/cdev_create.h>

NTSTATUS cdev_create(PDEVICE_OBJECT dev, PIRP irp) {
    UNREFERENCED_PARAMETER(dev);

    if (irp != NULL) {
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
 
   return STATUS_NOT_IMPLEMENTED;
}
