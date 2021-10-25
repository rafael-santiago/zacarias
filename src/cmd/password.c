/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/password.h>
#include <cmd/types.h>
#include <cmd/options.h>
#include <cmd/devio.h>
#include <cmd/utils.h>
#include <cmd/didumean.h>
#include <kbd/kbd.h>
#include <kryptos.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct  zc_data_drain_ctx {
    unsigned char *password;
    size_t password_size;
    unsigned char *pwdb_passwd;
    size_t pwdb_passwd_size;
};

static int zc_password_add(void);
static int zc_password_del(void);
static int zc_password_get(void);
static int zc_password_unk(void);

static void zc_drain_out_data(void *args);

static struct zc_exec_table_ctx g_zc_password_subcommands[] = {
    { "add", zc_password_add, NULL },
    { "del", zc_password_del, NULL },
    { "get", zc_password_get, NULL },
};

static size_t g_zc_password_subcommands_nr = sizeof(g_zc_password_subcommands) / sizeof(g_zc_password_subcommands[0]);

int zc_password(void) {
    zc_cmd_func sb_cmd = zc_password_unk;
    struct zc_exec_table_ctx *zc_pscmd = NULL;
    struct zc_exec_table_ctx *zc_pscmd_end = NULL;
    char *subcommand = NULL;
    const char **avail_subcommands = NULL;
    char **suggestions = NULL, **psug = NULL, **psug_end = NULL;
    size_t avail_subcommands_nr = 0;

    subcommand = zc_get_subcommand();

    if (subcommand == NULL) {
        fprintf(stderr, "ERROR: subcommand not informed.\n");
        return EXIT_FAILURE;
    }

    zc_pscmd = &g_zc_password_subcommands[0];
    zc_pscmd_end = zc_pscmd + g_zc_password_subcommands_nr;

    while (zc_pscmd != zc_pscmd_end && sb_cmd == zc_password_unk) {
        if (strcmp(zc_pscmd->cmd_name, subcommand) == 0) {
            sb_cmd = zc_pscmd->cmd_do;
        }
        zc_pscmd++;
    }

    if (sb_cmd == NULL) {
        fprintf(stderr, "ERROR: unknown password subcommand.\n");
        avail_subcommands = (const char **) malloc(sizeof(char **) * g_zc_password_subcommands_nr);
        suggestions = (char **) malloc(sizeof(char **) * g_zc_password_subcommands_nr);

        if (avail_subcommands != NULL && suggestions != NULL) {
            while (avail_subcommands_nr < g_zc_password_subcommands_nr) {
                avail_subcommands[avail_subcommands_nr] = g_zc_password_subcommands[avail_subcommands_nr].cmd_name;
                avail_subcommands_nr++;
            }

            didumean(subcommand, suggestions, avail_subcommands_nr, avail_subcommands, avail_subcommands_nr, 2);

            if (suggestions[0] != NULL) {
                fprintf(stderr, "\nDid you mean ");
                psug = suggestions;
                psug_end = psug + avail_subcommands_nr;
                while (psug != psug_end && *psug != NULL) {
                    fprintf(stderr, "'%s'%s", *psug, (((psug + 1) == psug_end || *(psug + 1) == NULL) ? "?\n" : ", "));
                    psug++;
                }
            }
        }

        if (avail_subcommands != NULL) {
            free(avail_subcommands);
        }

        if (suggestions != NULL) {
            free(suggestions);
        }
        return EXIT_FAILURE;
    }

    return sb_cmd();
}

int zc_password_help(void) {
    fprintf(stdout, "use: zc password add --user=<name> --alias=<name> [--sessioned]\n"
                    "     zc password del --user=<name> --alias=<name> [--sessioned]\n"
                    "     zc password get --user=<name> --alias=<name> [--timeout=<seconds>]\n");
    return EXIT_SUCCESS;
}

static int zc_password_unk(void) {
    fprintf(stderr, "ERROR: unknown password subcommand.\n");
    return 1;
}

static int zc_password_add(void) {
    zc_dev_t zcd = zcdev_open();
    char *user = NULL, *alias = NULL;
    int err = EXIT_FAILURE;
    size_t user_size = 0, alias_size = 0;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size = 0;
    unsigned char *session_passwd = NULL;
    size_t session_passwd_size = 0;
    unsigned char *password[2];
    size_t password_size[2];
    zc_device_status_t status;

    if (zcd == ZC_INVALID_DEVICE) {
        err = errno;
        goto zc_password_add_epilogue;
    }

    password[0] = password[1] = NULL;
    password_size[0] = password_size[1] = 0;

    ZC_GET_OPTION_OR_DIE(user, "user", zc_password_add_epilogue);
    user_size = strlen(user);

    ZC_GET_OPTION_OR_DIE(alias, "alias", zc_password_add_epilogue);
    alias_size = strlen(alias);

    fprintf(stdout, "Pwdb password: ");
    pwdb_passwd = zacarias_getuserkey(&pwdb_passwd_size);
    del_scr_line();

    if (pwdb_passwd == NULL || pwdb_passwd_size == 0) {
        fprintf(stderr, "ERROR: Null pwdb password.\n");
        goto zc_password_add_epilogue;
    }

    if (zc_get_bool_option("sessioned", 0)) {
        fprintf(stdout, "Session password: ");
        session_passwd = zacarias_getuserkey(&session_passwd_size);
        del_scr_line();
        if (session_passwd == NULL || session_passwd_size == 0) {
            fprintf(stderr, "ERROR: Null session password.\n");
            goto zc_password_add_epilogue;
        }
    }

    if (zc_get_bool_option("generate", 0) == 0) {
        fprintf(stdout, "Password: ");
        password[0] = zacarias_getuserkey(&password_size[0]);
        del_scr_line();

        if (password[0] == NULL || password_size[0] == 0) {
            fprintf(stderr, "ERROR: Null password.\n");
            goto zc_password_add_epilogue;
        }

        fprintf(stdout, "Confirm your password choice by re-typing it: ");
        password[1] = zacarias_getuserkey(&password_size[1]);
        del_scr_line();

        if (password[1] == NULL || password_size[1] == 0) {
            fprintf(stderr, "ERROR: Null password confirmation.\n");
            goto zc_password_add_epilogue;
        }

        if (password_size[0] != password_size[1] || memcmp(password[0], password[1], password_size[0]) != 0) {
            fprintf(stderr, "ERROR: Informed passwords do not match.\n");
            goto zc_password_add_epilogue;
        }
    }

    err = zcdev_add_password(zcd, user, user_size, pwdb_passwd, pwdb_passwd_size, session_passwd, session_passwd_size,
                             alias, alias_size, password[0], password_size[0], &status);

    if (err == 0 && status != kNoError) {
        zcdev_perror(status);
        err = EXIT_FAILURE;
    }

zc_password_add_epilogue:

    zcdev_close(zcd);

    user = alias = NULL;
    user_size = alias_size = 0;

    if (pwdb_passwd != NULL) {
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
        pwdb_passwd_size = 0;
    }

    if (session_passwd != NULL) {
        kryptos_freeseg(session_passwd, session_passwd_size);
        session_passwd_size = 0;
    }

    if (password[0] != NULL) {
        kryptos_freeseg(password[0], password_size[0]);
        password_size[0] = 0;
    }

    if (password[1] != NULL) {
        kryptos_freeseg(password[1], password_size[1]);
        password_size[1] = 0;
    }

    return err;
}

static int zc_password_del(void) {
    zc_dev_t zcd = zcdev_open();
    char *user = NULL, *alias = NULL;
    unsigned char *pwdb_passwd = NULL;
    unsigned char *session_passwd = NULL;
    size_t user_size = 0, alias_size = 0, pwdb_passwd_size = 0, session_passwd_size = 0;
    int err = 0;
    zc_device_status_t status;

    if (zcd == - 1) {
        err = errno;
        goto zc_password_del_epilogue;
    }

    ZC_GET_OPTION_OR_DIE(user, "user", zc_password_del_epilogue);
    user_size = strlen(user);

    ZC_GET_OPTION_OR_DIE(alias, "alias", zc_password_del_epilogue);
    alias_size = strlen(alias);

    fprintf(stdout, "Pwdb password: ");
    pwdb_passwd = zacarias_getuserkey(&pwdb_passwd_size);
    del_scr_line();

    if (pwdb_passwd == NULL || pwdb_passwd_size == 0) {
        fprintf(stderr, "ERROR: Null pwdb password.\n");
        goto zc_password_del_epilogue;
    }

    if (zc_get_bool_option("sessioned", 0)) {
        fprintf(stdout, "Session password: ");
        session_passwd = zacarias_getuserkey(&session_passwd_size);
        del_scr_line();
        if (session_passwd == NULL || session_passwd_size == 0) {
            fprintf(stderr, "ERROR: Null session password.\n");
            goto zc_password_del_epilogue;
        }
    }

    err = zcdev_del_password(zcd, user, user_size, pwdb_passwd, pwdb_passwd_size, session_passwd, session_passwd_size,
                             alias, alias_size, &status);

    if (err == 0 && status != kNoError) {
        zcdev_perror(status);
        err = 1;
    }

zc_password_del_epilogue:

    zcdev_close(zcd);

    user = alias = NULL;
    user_size = alias_size = 0;

    if (pwdb_passwd != NULL) {
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
        pwdb_passwd_size = 0;
    }

    if (session_passwd != NULL) {
        kryptos_freeseg(session_passwd, session_passwd_size);
        session_passwd_size = 0;
    }

    return err;
}

static int zc_password_get(void) {
    zc_dev_t zcd = zcdev_open();
    char *user = NULL, *alias = NULL, *timeout = NULL, *tp, *tp_end;
    unsigned char *password = NULL, *pwdb_passwd = NULL;
    size_t user_size = 0, alias_size = 0, password_size = 0, pwdb_passwd_size = 0;
    int err = EXIT_FAILURE;
    zc_device_status_t status;
    struct zc_data_drain_ctx zc_drain;

    if (zcd == ZC_INVALID_DEVICE) {
        err = errno;
        goto zc_password_get_epilogue;
    }

    memset(&zc_drain, 0, sizeof(zc_drain));

    if (!zacarias_set_kbd_layout("pt-br")) {
        fprintf(stderr, "ERROR: Unable to set internal keyboard layout.\n");
        goto zc_password_get_epilogue;
    }

    ZC_GET_OPTION_OR_DIE(user, "user", zc_password_get_epilogue);
    user_size = strlen(user);

    ZC_GET_OPTION_OR_DIE(alias, "alias", zc_password_get_epilogue);
    alias_size = strlen(alias);

    timeout = zc_get_option("timeout", "5");
    tp = &timeout[0];
    tp_end = tp + strlen(tp);
    if (tp == tp_end) {
        fprintf(stderr, "ERROR: Null timeout.\n");
        err = 1;
        goto zc_password_get_epilogue;
    }

    while (tp != tp_end) {
        if (!isdigit(*tp)) {
            fprintf(stderr, "ERROR: Invalid timeout : '%s'.\n", timeout);
            err = 1;
            goto zc_password_get_epilogue;
        }
        tp++;
    }

    fprintf(stdout, "Pwdb password: ");
    pwdb_passwd = zacarias_getuserkey(&pwdb_passwd_size);
    del_scr_line();

    if (pwdb_passwd == NULL || pwdb_passwd_size == 0) {
        fprintf(stderr, "ERROR: Null pwdb password.\n");
        goto zc_password_get_epilogue;
    }

    err = zcdev_get_password(zcd, user, user_size, pwdb_passwd, pwdb_passwd_size,
                             alias, alias_size, &password, &password_size, &status);

    if (err == 0 && status != kNoError) {
        zcdev_perror(status);
        err = 1;
        goto zc_password_get_epilogue;
    }

    if (password == NULL || password_size == 0) {
        fprintf(stderr, "PANIC: Ohhhh... Hey Beavis, Huh! Password must not be null but it seems to be!!! "
                        "Huh-huh... \"- Nulllll\" he-he-he yeah-yeah huh-huh-huh! It smells like developer team shitspirt! "
                        "he-he-huh-huh yeah-yeah!\n");
        err = 1;
        goto zc_password_get_epilogue;
    }

    // INFO(Rafael): Pretty important step. Otherwise we could leak passwords when attending impatient users.
    zc_drain.password = password;
    zc_drain.password_size = password_size;
    zc_drain.pwdb_passwd = pwdb_passwd;
    zc_drain.pwdb_passwd_size = pwdb_passwd_size;

    fprintf(stdout, "INFO: Now put your cursor focus where your password must be typed and wait.");
    fflush(stdout);
    err = zacarias_sendkeys(password, password_size, atoi(timeout), zc_drain_out_data, &zc_drain);

    del_scr_line();

    if (err != 0) {
        fprintf(stderr, "ERROR: While trying to automagically type your password at the area where you put your cursor.\n");
    }

    memset(&zc_drain, 0, sizeof(zc_drain));

zc_password_get_epilogue:

    user = alias = timeout = NULL;
    user_size = alias_size = 0;

    if (pwdb_passwd != NULL) {
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
        pwdb_passwd_size = 0;
    }

    if (password != NULL) {
        kryptos_freeseg(password, password_size);
        password_size = 0;
    }

    signal(SIGINT | SIGTERM, NULL);

    return err;
}

static void zc_drain_out_data(void *args) {
    struct zc_data_drain_ctx *zc_drain = (struct zc_data_drain_ctx *)args;
    if (zc_drain != NULL) {
        kryptos_freeseg(zc_drain->pwdb_passwd, zc_drain->pwdb_passwd_size);
        zc_drain->pwdb_passwd_size = 0;
        kryptos_freeseg(zc_drain->password, zc_drain->password_size);
        zc_drain->password_size = 0;
    }
    fprintf(stdout, "\r                                                                          "
                    "\r-- Aborted by the user. Anyway, all sensitive data was cleared.\n");
    exit(1);
}
