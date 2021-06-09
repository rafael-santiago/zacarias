#include <cmd/detach.h>
#include <cmd/options.h>
#include <cmd/devio.h>
#include <kbd/kbd.h>
#include <kryptos.h>
#include <stdio.h>

int zc_detach(void) {
    int zcd = zcdev_open();
    int err = 1;
    char *user = NULL;
    size_t user_size = 0;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size = 0;
    zc_device_status_t status;

    if (zcd == -1) {
        goto zc_detach_epilogue;
    }

    ZC_GET_OPTION_OR_DIE(user, "user", zc_detach_epilogue);
    user_size = strlen(user);

    fprintf(stdout, "Pwdb password: ");
    pwdb_passwd = zacarias_getuserkey(&pwdb_passwd_size);
    del_scr_line();

    if (pwdb_passwd == NULL || pwdb_passwd_size == 0) {
        fprintf(stderr, "ERROR: Null pwdb password.\n");
        goto zc_detach_epilogue;
    }

    err = zcdev_detach(zcd, user, user_size, pwdb_passwd, pwdb_passwd_size, &status);

    if (err == 0 && status != kNoError) {
        err = 1;
        zcdev_perror(status);
    }

zc_detach_epilogue:

    if (pwdb_passwd != NULL) {
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
    }

    zcdev_close(zcd);

    return err;
}

int zc_detach_help(void) {
    fprintf(stdout, "use: zc detach --user=<name>\n");
    return 0;
}
