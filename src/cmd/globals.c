/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/types.h>
#include <cmd/version.h>
#include <cmd/attach.h>
#include <cmd/detach.h>
#include <cmd/password.h>
#include <cmd/device.h>
#include <cmd/help.h>

#define ZC_REGISTER_COMMAND(name) { #name, zc_ ##name, zc_ ##name ##_help }

struct zc_exec_table_ctx g_command_table[] = {
    ZC_REGISTER_COMMAND(attach),
    ZC_REGISTER_COMMAND(detach),
    ZC_REGISTER_COMMAND(password),
    ZC_REGISTER_COMMAND(device),
    ZC_REGISTER_COMMAND(help),
    ZC_REGISTER_COMMAND(version),
};

size_t g_command_table_nr = sizeof(g_command_table) / sizeof(g_command_table[0]);

#undef ZC_REGISTER_COMMAND
