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
#include <sys/proc.h>

int cdev_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data, int flag __unused, struct thread *td) {
    struct zc_devio_ctx *dev_p = NULL;
    int error = 0;

    if (data == NULL) {
        error = EFAULT;
        goto cdev_ioctl_epilogue;
    }

    dev_p = (struct zc_devio_ctx *)data;

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

        case ZACARIAS_ALIASES:
            error = zc_dev_act_aliases(&dev_p);
            break;

        default:
            ZC_DBG("Unknown command received.\n");
            dev_p->status = kUnknownDeviceCommand;
            break;
    }

    td->td_retval[0] = error;

cdev_ioctl_epilogue:

    return error;
}
