#include <cmd/attach.h>
#include <cmd/options.h>
#include <cmd/devio.h>
#include <kbd/kbd.h>
#include <kryptos.h>
#include <string.h>
#include <stdio.h>

int zc_attach(void) {
    int zcd = zcdev_open();
    int err = 0;
    char *pwdb_path = NULL;
    size_t pwdb_path_size = 0;
    char *user = NULL;
    size_t user_size = 0;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size = 0;
    unsigned char *session_passwd[2] = { NULL, NULL };
    size_t session_passwd_size[2] = { 0, 0 };
    zc_device_status_t status;

    if (zcd == -1) {
        err = errno;
        goto zc_attach_epilogue;
    }

    ZC_GET_OPTION_OR_DIE(pwdb_path, "pwdb", zc_attach_epilogue);
    pwdb_path_size = strlen(pwdb_path);

    ZC_GET_OPTION_OR_DIE(user, "user", zc_attach_epilogue);

    fprintf(stdout, "Pwdb password: ");
    pwdb_passwd = zacarias_getuserkey(&pwdb_passwd_size);

    if (pwdb_passwd == NULL || pwdb_passwd_size == 0) {
        fprintf(stderr, "ERROR: Null pwdb password.\n");
        goto zc_attach_epilogue;
    }

    if (zc_get_bool_option("sessioned", 0)) {
        fprintf(stdout, "Session password: ");
        session_passwd[0] = zacarias_getuserkey(&session_passwd_size[0]);

        if (session_passwd[0] == NULL || session_passwd_size[0] == 0) {
            fprintf(stderr, "ERROR: Null session password.\n");
            goto zc_attach_epilogue;
        }

        fprintf(stdout, "Confirm the session password: ");
        session_passwd[1] = zacarias_getuserkey(&session_passwd_size[1]);

        if (session_passwd[1] == NULL || session_passwd_size[1] == 0) {
            fprintf(stderr, "ERROR: Null session password confirmation.\n");
            goto zc_attach_epilogue;
        }

        if (session_passwd_size[0] != session_passwd_size[1] ||
            memcmp(session_passwd[0], session_passwd[1], session_passwd_size[0]) != 0) {
            fprintf(stderr, "ERROR: Session password does not match with its confirmation.\n");
            goto zc_attach_epilogue;
        }
    }

    err = zcdev_attach(zcd, pwdb_path, pwdb_path_size,
                       user, user_size,
                       pwdb_passwd, pwdb_passwd_size,
                       session_passwd[0], session_passwd_size[0], &status);


    if (err == 0 && status != 0) {
        zcdev_perror(status);
        err = 1;
    }

zc_attach_epilogue:

    if (pwdb_passwd != NULL) {
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
    }

    if (session_passwd[0] != NULL) {
        kryptos_freeseg(session_passwd[0], session_passwd_size[0]);
    }

    if (session_passwd[1] != NULL) {
        kryptos_freeseg(session_passwd[1], session_passwd_size[1]);
    }

    return err;
}

int zc_attach_help(void) {
    fprintf(stdout, "use: zc attach --pwdb=<profile path> --user=<name> [--sessioned]\n\r");
}
