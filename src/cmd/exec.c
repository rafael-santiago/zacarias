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
#include <kryptos_random.h>
#include <aegis.h>
#if defined(__unix__)
# include <unistd.h>
# include <termios.h>
# include <fcntl.h>
# if defined(_POSIX_MEMLOCK)
#  include <sys/mman.h>
# endif
#endif
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(__unix__)
static struct termios gOriginal_tty_conf;
#endif

static int gZcExiting = 0;

static int zc_unk_command(void);

static int zc_should_disable_dbg_gorgon(void *args);

static void zc_on_debugger_attachment(void *args);

static void zc_sigint_watchdog(int signo);

#if defined(__unix__)
static void zc_tr_abait(void);
#endif

int zc_exec(const int argc, char **argv) {
    zc_cmd_func zc_cmd = zc_unk_command;
    struct zc_exec_table_ctx *zetc, *zetc_end;
    char *cmd_name = "";
    char buf[1];

#if defined(__unix__)
    tcgetattr(STDOUT_FILENO, &gOriginal_tty_conf);
#endif

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

#if defined(__unix__)
    // INFO(Rafael): A kind of no operation only used to trigger syscall tracing before any sensitve stuff.
    zc_tr_abait();
#endif

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
#if defined(__unix__)
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &gOriginal_tty_conf);
#endif
    fprintf(stderr, "ALERT: A debugger attachment was detect. Aborting execution to avoid more damage.\n"
                    "       Do not execute zc again until make sure that your system is clean.\n");
    exit(EXIT_FAILURE);
}

static void zc_sigint_watchdog(int signo) {
    gZcExiting = 1;
}

#if defined(__unix__)
void zc_tr_abait(void) {
    int fd = -1;
    size_t times_nr = 1, t;
    size_t bytes_nr = 0;
    char buf[256];
    if ((fd = open("/dev/urandom", O_RDONLY)) == -1) {
        fprintf(stderr, "ALERT: zc_tr_abait() has failed.\n"
                        "       It is a little suspicious, better to abort execution here.\n");
        exit(1);
    }
    while ((t = kryptos_unbiased_rand_mod_u8(10)) == 0)
        ;
    for (t = 0; t < times_nr; t++) {
        while((bytes_nr = kryptos_unbiased_rand_mod_u8(50)) == 0)
            ;
        read(fd, buf, bytes_nr);
        usleep(50);
    }
    close(fd);
}
#endif
