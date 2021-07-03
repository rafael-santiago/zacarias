/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_CMD_DEVIO_H
#define ZACARIAS_CMD_DEVIO_H 1

#include <dev/defs/io.h>
#include <unistd.h>
#include <errno.h>

int zcdev_open(void);

#define zcdev_close(fd) {\
    if ((fd) > -1) {\
        close((fd));\
    }\
}

int zcdev_attach(const int zcd,
                 const char *pwdb_path, const size_t pwdb_path_size,
                 const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 const unsigned char *session_passwd, const size_t session_passwd_size,
                 const int init, zc_device_status_t *status);

int zcdev_detach(const int zcd, const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 zc_device_status_t *status);

int zcdev_add_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const unsigned char *session_passwd, const size_t session_passwd_size,
                       const char *alias, const size_t alias_size,
                       const unsigned char *password, const size_t password_size,
                       zc_device_status_t *status);

int zcdev_del_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const unsigned char *session_passwd, const size_t session_passwd_size,
                       const char *alias, const size_t alias_size,
                       zc_device_status_t *status);

int zcdev_get_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const char *alias, const size_t alias_size,
                       unsigned char **password, size_t *password_size,
                       zc_device_status_t *status);

void zcdev_perror(const zc_device_status_t status);

#endif
