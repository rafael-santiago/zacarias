/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/exec.h>
#include <cmd/types.h>
#include <cmd/options.h>
#include <kryptos_memory.h>
#if defined(__unix__)
# include <unistd.h>
# if defined(_POSIX_MEMLOCK)
#  include <sys/mman.h>
# endif
#endif
#include <string.h>
#include <stdio.h>

static int zc_unk_command(void);

int zc_exec(const int argc, char **argv) {
    zc_cmd_func zc_cmd = zc_unk_command;
    struct zc_exec_table_ctx *zetc, *zetc_end;
    char *cmd_name = "";

    kryptos_avoid_ram_swap();

#if defined(__unix__) && defined(_POSIX_MEMLOCK)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        perror("mlockall");
        return EXIT_FAILURE;
    }
#endif

    zc_set_argc_argv(argc, argv);

    if ((cmd_name = zc_get_command()) == NULL) {
        fprintf(stderr, "ERROR: No command was passed.\n");
        return 1;
    }

    if (strcmp(cmd_name, "--version") == 0) {
        cmd_name = "version";
    }

    zetc = &g_command_table[0];
    zetc_end = zetc + g_command_table_nr;

    while (zetc != zetc_end && zc_cmd == zc_unk_command) {
        if (strcmp(zetc->cmd_name, cmd_name) == 0) {
            zc_cmd = zetc->cmd_do;
        }
        zetc++;
    }

    return zc_cmd();
}

static int zc_unk_command(void) {
    fprintf(stderr, "ERROR: unknown command.\n");
    return 1;
}
