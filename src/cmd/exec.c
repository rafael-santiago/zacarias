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
#include <aegis.h>
#if defined(__unix__)
# include <unistd.h>
# if defined(_POSIX_MEMLOCK)
#  include <sys/mman.h>
# endif
#endif
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int gZcExiting = 0;

static int zc_unk_command(void);

static int zc_should_disable_dbg_gorgon(void *args);

static void zc_on_debugger_attachment(void *args);

static void zc_sigint_watchdog(int signo);

int zc_exec(const int argc, char **argv) {
    zc_cmd_func zc_cmd = zc_unk_command;
    struct zc_exec_table_ctx *zetc, *zetc_end;
    char *cmd_name = "";

    signal(SIGINT, zc_sigint_watchdog);
    signal(SIGTERM, zc_sigint_watchdog);

    kryptos_avoid_ram_swap();

    if (aegis_set_gorgon(zc_should_disable_dbg_gorgon,
                         &gZcExiting,
                         zc_on_debugger_attachment,
                         NULL) != 0) {
        fprintf(stderr, "ERROR: Unable to set anti-debugging mechanism.\n");
        return EXIT_FAILURE;
    }

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
    return EXIT_FAILURE;
}


static int zc_should_disable_dbg_gorgon(void *args) {
    return (*(int *)args);
}

static void zc_on_debugger_attachment(void *args) {
    fprintf(stderr, "ALERT: A debugger attachment was detect. Aborting execution to avoid more damage.\n"
                    "       Do not execute zc again until make sure that your system is clean.\n");
    exit(EXIT_FAILURE);
}

static void zc_sigint_watchdog(int signo) {
    gZcExiting = 1;
}
