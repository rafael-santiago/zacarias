/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/cdev_ioctl.h>
#include <actions.h>
#include <defs/io.h>
#include <defs/zc_dbg.h>

NTSTATUS cdev_ioctl(PDEVICE_OBJECT dev, PIRP irp) {
    UNREFERENCED_PARAMETER(dev);

    PIO_STACK_LOCATION ioslp = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS status = STATUS_SUCCESS;
    struct zc_devio_ctx *dev_p = NULL;

    if (ioslp == NULL) {
        return STATUS_UNEXPECTED_IO_ERROR;
    }

    if (ioslp->Parameters.DeviceIoControl.InputBufferLength != sizeof(struct zc_devio_ctx) ||
        irp->AssociatedIrp.SystemBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    dev_p = (struct zc_devio_ctx *)irp->AssociatedIrp.SystemBuffer;

    switch (ioslp->Parameters.DeviceIoControl.IoControlCode) {
        case ZACARIAS_ATTACH_PROFILE:
            status = zc_dev_act_attach_profile(&dev_p);
            break;

        case ZACARIAS_DETACH_PROFILE:
            status = zc_dev_act_detach_profile(&dev_p);
            break;

        case ZACARIAS_ADD_PASSWORD:
            status = zc_dev_act_add_password(&dev_p);
            break;

        case ZACARIAS_DEL_PASSWORD:
            status = zc_dev_act_del_password(&dev_p);
            break;

        case ZACARIAS_GET_PASSWORD:
            status = zc_dev_act_get_password(&dev_p);
            break;

        default:
            ZC_DBG("Unknown command received\n");
            dev_p->status = kUnknownDeviceCommand;
            break;
    }

    if (irp != NULL) {
        irp->IoStatus.Information = sizeof(struct zc_devio_ctx);
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
 
   return status;
}
