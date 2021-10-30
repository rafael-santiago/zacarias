/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_CMD_TYPES_H
#define ZACARIAS_CMD_TYPES_H 1

#include <stdlib.h>

typedef int (*zc_cmd_func)(void);

#if defined(__unix__)
  typedef int zc_dev_t;
# define ZC_INVALID_DEVICE -1
#elif defined(_WIN32)
# include <windows.h>
  typedef HANDLE zc_dev_t;
# define ZC_INVALID_DEVICE INVALID_HANDLE_VALUE
# define ZACARIAS_SERVICE "dev_zacarias"
# define ZACARIAS_DISPLAY_NAME "ZacariasDevice"
# define ZACARIAS_DEVICE_LINK "\\\\.\\ZacariasDeviceLink"
#else
# error Some code wanted.
#endif

struct zc_exec_table_ctx {
    const char *cmd_name;
    zc_cmd_func cmd_do, cmd_help;
};

extern struct zc_exec_table_ctx g_command_table[];

extern size_t g_command_table_nr;

#define ZC_VERSION "0.0.1"

#endif
