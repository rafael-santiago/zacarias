/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <freebsd/cdev_ioctl.h>
#include <defs/io.h>
#include <defs/zc_dbg.h>
#include <defs/types.h>
#include <actions.h>

int cdev_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data, int flag __unused, struct thread *td __unused) {
    struct zc_devio_ctx devio = { 0 }, *dev_p = &devio;
    int error = 0;

    if (data == NULL) {
        error = EFAULT;
        goto cdev_ioctl_epilogue;
    }

    if (kcpy(&devio, (struct zc_devio_ctx *)data, sizeof(struct zc_devio_ctx)) != 0) {
        error = EFAULT;
        goto cdev_ioctl_epilogue;
    }

    switch (cmd) {
        case ZACARIAS_ATTACH_PROFILE:
            error = zc_dev_act_attach_profile(&dev_p);
            break;

        case ZACARIAS_DETACH_PROFILE:
            error = zc_dev_act_detach_profile(&dev_p);
            break;

        case ZACARIAS_ADD_PASSWORD:
            error = zc_dev_act_add_password(&dev_p);
            break;

        case ZACARIAS_DEL_PASSWORD:
            error = zc_dev_act_del_password(&dev_p);
            break;

        case ZACARIAS_GET_PASSWORD:
            error = zc_dev_act_get_password(&dev_p);
            break;

        default:
            ZC_DBG("Unknown command received.\n");
            devio.status = kUnknownDeviceCommand;
            break;
    }

    if (ucpy((void *)data, &devio, sizeof(struct zc_devio_ctx)) != 0) {
        ZC_DBG("ucpy() has failed.\n");
        error = EFAULT;
    }

cdev_ioctl_epilogue:

    return error;
}
