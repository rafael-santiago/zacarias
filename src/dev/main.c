/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#define ZACARIAS_DEV_VERSION "0.0.1"

#if defined(__linux__)

#include <linux/cdev_init.h>
#include <linux/cdev_deinit.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Santiago");
MODULE_DESCRIPTION("Zacarias password manager char device");
MODULE_VERSION(ZACARIAS_DEV_VERSION);

static int __init ini(void) {
    return zcdev_init();
}

static void __exit finis(void) {
    zcdev_deinit();
}

module_init(ini);
module_exit(finis);

#elif defined(__FreeBSD__)

#include <freebsd/cdev_init.h>
#include <freebsd/cdev_deinit.h>
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/conf.h>

static int zacarias_modevent(module_t mod __unused, int event, void *arg __unused) {
    int error = 0;

    switch (event) {
        case MOD_LOAD:
            error = cdev_init();
            break;

        case MOD_QUIESCE:
            break;

        case MOD_UNLOAD:
            error = cdev_deinit();
            break;

        default:
            error = EOPNOTSUPP;
            break;
    }

    return error;
}

DEV_MODULE(zacarias, zacarias_modevent, NULL);

#elif defined(_WIN32)

#include <windows/cdev_init.h>
#include <windows/cdev_deinit.h>

void DriverUnload(_In_ PDRIVER_OBJECT driver_object) {
    UNREFERENCED_PARAMETER(driver_object);
    cdev_deinit();
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driver_object, _In_ PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(registry_path);
    driver_object->DriverUnload = DriverUnload;
    return cdev_init(driver_object);
}

#else
# error Some code wanted
#endif
