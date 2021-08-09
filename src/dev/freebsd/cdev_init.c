/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <freebsd/cdev_init.h>
#include <freebsd/cdev_open.h>
#include <freebsd/cdev_ioctl.h>
#include <defs/types.h>
#include <defs/io.h>
#include <ctx/ctx.h>
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/syscall.h>

static struct cdevsw zacarias_cdevsw = {
    .d_version = D_VERSION,
    .d_open = cdev_open,
    .d_ioctl = cdev_ioctl,
    .d_name = CDEVNAME
};

int cdev_init(void) {
    int error = 0;

    cdev_mtx_init(&g_cdev()->lock);

    zacarias_profiles_ctx_init(g_cdev()->profiles);

    g_cdev()->device = make_dev(&zacarias_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666, CDEVNAME);
    if (g_cdev()->device == NULL) {
        cdev_mtx_deinit(&g_cdev()->lock);
        error = EFAULT;
    }

    if (error == 0) {
        uprintf("/dev/zacarias: Device Initialized.\n");
    }

    return error;
}
