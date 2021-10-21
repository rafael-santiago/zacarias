/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/cdev_create.h>
#include <defs/zc_dbg.h>

NTSTATUS cdev_create(PDEVICE_OBJECT dev, PIRP irp) {
    UNREFERENCED_PARAMETER(dev);
    NTSTATUS status = STATUS_DEVICE_BUSY;

    if (cdev_mtx_trylock(&g_cdev()->lock)) {
        status = STATUS_SUCCESS;
        cdev_mtx_unlock(&g_cdev()->lock);
    } else {
        ZC_DBG("return EBUSY.\n");
    }

    if (irp != NULL) {
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
 
   return status;
}
