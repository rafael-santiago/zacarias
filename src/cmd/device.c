/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/device.h>
#include <cmd/types.h>
#include <cmd/options.h>
#if defined(__linux__)
# include <sys/syscall.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int zc_device_install(void);
static int zc_device_uninstall(void);
static int zc_device_unk(void);

static struct zc_exec_table_ctx g_zc_device_subcommands[] = {
    { "install", zc_device_install, NULL },
    { "uninstall", zc_device_uninstall, NULL },
};

static size_t g_zc_device_subcommands_nr = sizeof(g_zc_device_subcommands) / sizeof(g_zc_device_subcommands[0]);

int zc_device(void) {
    zc_cmd_func sb_cmd = zc_device_unk;
    char *subcommand = zc_get_subcommand();
    struct zc_exec_table_ctx *zc_etc = NULL, *zc_etc_end = NULL;

    if (subcommand == NULL) {
        fprintf(stderr, "ERROR: subcommand not informed.\n");
        return EXIT_FAILURE;
    }

    zc_etc = &g_zc_device_subcommands[0];
    zc_etc_end = zc_etc + g_zc_device_subcommands_nr;

    while (zc_etc != zc_etc_end && sb_cmd == zc_device_unk) {
        if (strcmp(zc_etc->cmd_name, subcommand) == 0) {
            sb_cmd = zc_etc->cmd_do;
        }
        zc_etc++;
    }

    return sb_cmd();
}

int zc_device_help(void) {
    fprintf(stdout, "use: zc device install --device-driver-path=<filepath>\n"
                    "     zc device uninstall\n");
    return EXIT_SUCCESS;
}

static int zc_device_install(void) {
    int err = EXIT_FAILURE;
    char *device_driver_path = NULL;
#if defined(__linux__)
    int fd = -1;
# define init_linux_mod(fd) syscall(__NR_finit_module, fd, "", 0)
    ZC_GET_OPTION_OR_DIE(device_driver_path, "device-driver-path", zc_device_install_epilogue);

    if ((fd = open(device_driver_path, O_RDONLY)) == -1) {
        fprintf(stderr, "ERROR: Unable to read module from '%s'.\n", device_driver_path);
        goto zc_device_install_epilogue;
    }

    if (init_linux_mod(fd) != 0) {
        fprintf(stderr, "ERROR: While trying to install kernel module '%s'.\n", device_driver_path);
    } else {
        err = EXIT_SUCCESS;
    }

    close(fd);
# undef init_linux_mod
#else
# error Some code wanted.
#endif

zc_device_install_epilogue:
    return err;
}

static int zc_device_uninstall(void) {
    int err = EXIT_FAILURE;
#if defined(__linux__)
# define deinit_linux_mod(name) syscall(__NR_delete_module, name, 0)
    if ((err = deinit_linux_mod("zacarias")) != 0) {
        fprintf(stderr, "ERROR: Unable to uninstall kernel module.\n");
    }
# undef deinit_linux_mod
#else
# error Some code wanted.
#endif
    return err;
}

static int zc_device_unk(void) {
    fprintf(stderr, "ERROR: unknown device subcommand.\n");
    return EXIT_FAILURE;
}