/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/options.h>
#include <cmd/types.h>
#include <string.h>
#include <stdio.h>

static int g_argc;

static char **g_argv = NULL;

void zc_set_argc_argv(const int argc, char **argv) {
    g_argc = argc;
    g_argv = argv;
}

char *zc_get_raw_arg(const size_t arg_nr) {
    if (g_argv == NULL || arg_nr > g_argc) {
        return NULL;
    }
    return &g_argv[arg_nr][0];
}

char *zc_get_option(const char *option, char *default_value) {
    char **arg, **arg_end;
    char temp[4096];

    if (option == NULL || g_argv == NULL || g_argc == 0) {
        return default_value;
    }

    snprintf(temp, sizeof(temp) - 1, "--%s=", option);

    arg = g_argv;
    arg_end = arg + g_argc;

    while (arg != arg_end) {
        if (strstr(*arg, temp) == *arg) {
            return (&(*arg)[0] + strlen(temp));
        }
        arg++;
    }

    return default_value;
}

int zc_get_bool_option(const char *option, const int default_value) {
    char **arg, **arg_end;
    char temp[4096];

    if (option == NULL || g_argv == NULL || g_argc == 0) {
        return default_value;
    }

    snprintf(temp, sizeof(temp) - 1, "--%s", option);

    arg = g_argv;
    arg_end = arg + g_argc;

    while (arg != arg_end) {
        if (strcmp(*arg, temp) == 0) {
            return 1;
        }
        arg++;
    }

    return default_value;

}

char *zc_get_command(void) {
    if (g_argv == NULL || g_argc < 2) {
        return NULL;
    }
    return &g_argv[1][0];
}

char *zc_get_subcommand(void) {
    if (g_argv == NULL || g_argc < 3) {
        return NULL;
    }
    return &g_argv[2][0];
}
