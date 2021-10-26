/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/devio.h>
#include <cmd/utils.h>
#include <kryptos.h>
#include <string.h>
#if defined(__unix__)
# include <sys/ioctl.h>
# include <fcntl.h>
#endif
#include <stdlib.h>
#include <stdio.h>

static int zcdev_ioctl(const zc_dev_t zcd, const unsigned long cmd, struct zc_devio_ctx *ioctx);

zc_dev_t zcdev_open(void) {
#if defined(__unix__)
    zc_dev_t zcd = open("/dev/"CDEVNAME, O_RDWR);
    int ntry = 10;

    while (zcd == -1 && ntry-- > 0) {
        sleep(1);
        zcd = open(CDEVNAME, O_RDWR);
    }

    if (zcd == -1) {
        fprintf(stderr, "ERROR: Unable to open zacarias device : ");
        perror("failure detail : ");
        fprintf(stderr, "\n");
    }

    return zcd;
#elif defined(_WIN32)
    zc_dev_t zcd = CreateFileA(ZACARIAS_DEVICE_LINK, GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    int ntry = 10;
    char err_buf[1024];

    memset(err_buf, 0, sizeof(err_buf));

    while (zcd == INVALID_HANDLE_VALUE && ntry-- > 0) {
        Sleep(1000);
        zcd = CreateFileA(ZACARIAS_DEVICE_LINK, GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    }

    if (zcd == INVALID_HANDLE_VALUE) {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       err_buf,
                       sizeof(err_buf),
                       NULL);
        fprintf(stderr, "ERROR: Unable to open zacarias device : failure detail : %s\n", err_buf);
    }

    return INVALID_HANDLE_VALUE;
#else
# error Some code wanted.
#endif
}

int zcdev_attach(const zc_dev_t zcd,
                 const char *pwdb_path, const size_t pwdb_path_size,
                 const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 const unsigned char *session_passwd, const size_t session_passwd_size,
                 const int init, zc_device_status_t *status) {
    struct zc_devio_ctx ioctx;
    int err = EXIT_FAILURE;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = (!init) ? kAttachProfile : kInitAndAttachProfile;

    if (get_canonical_path(ioctx.pwdb_path, sizeof(ioctx.pwdb_path), pwdb_path, pwdb_path_size) == NULL) {
        fprintf(stderr, "error: PWDB path does not exist.\n");
        return EINVAL;
    }
    ioctx.pwdb_path_size = strlen(ioctx.pwdb_path);

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                           : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    ioctx.session_passwd_size = (session_passwd_size > sizeof(ioctx.session_passwd) - 1) ? sizeof(ioctx.session_passwd) - 1
                                                                                         : session_passwd_size;
    memcpy(ioctx.session_passwd, session_passwd, ioctx.session_passwd_size);

    ioctx.sessioned = (session_passwd != NULL);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_ATTACH_PROFILE, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

int zcdev_detach(const zc_dev_t zcd, const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 zc_device_status_t *status) {
    struct zc_devio_ctx ioctx;
    int err = EXIT_FAILURE;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = kDetachProfile;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_DETACH_PROFILE, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

int zcdev_add_password(const zc_dev_t zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const unsigned char *session_passwd, const size_t session_passwd_size,
                       const char *alias, const size_t alias_size,
                       const unsigned char *password, const size_t password_size,
                       zc_device_status_t *status) {
    int err = EXIT_FAILURE;
    struct zc_devio_ctx ioctx;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = kAddPassword;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    if (session_passwd != NULL) {
        ioctx.session_passwd_size = (session_passwd_size > sizeof(ioctx.session_passwd) - 1) ? sizeof(ioctx.session_passwd) - 1
                                                                                             : session_passwd_size;
        memcpy(ioctx.session_passwd, session_passwd, ioctx.session_passwd_size);
        ioctx.sessioned = 1;
    }

    ioctx.alias_size = (alias_size > sizeof(ioctx.alias) - 1) ? sizeof(ioctx.alias) - 1 : alias_size;
    memcpy(ioctx.alias, alias, ioctx.alias_size);

    ioctx.passwd_size = (password_size > sizeof(ioctx.passwd) - 1) ? sizeof(ioctx.passwd) - 1 : password_size;
    memcpy(ioctx.passwd, password, ioctx.passwd_size);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_ADD_PASSWORD, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

int zcdev_del_password(const zc_dev_t zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const unsigned char *session_passwd, const size_t session_passwd_size,
                       const char *alias, const size_t alias_size,
                       zc_device_status_t *status) {
    int err = EXIT_FAILURE;
    struct zc_devio_ctx ioctx;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = kDelPassword;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    if (session_passwd != NULL) {
        ioctx.session_passwd_size = (session_passwd_size > sizeof(ioctx.session_passwd) - 1) ? sizeof(ioctx.session_passwd) - 1
                                                                                             : session_passwd_size;
        memcpy(ioctx.session_passwd, session_passwd, ioctx.session_passwd_size);
        ioctx.sessioned = 1;
    }

    ioctx.alias_size = (alias_size > sizeof(ioctx.alias) - 1) ? sizeof(ioctx.alias) - 1 : alias_size;
    memcpy(ioctx.alias, alias, ioctx.alias_size);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_DEL_PASSWORD, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

int zcdev_get_password(const zc_dev_t zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const char *alias, const size_t alias_size,
                       unsigned char **password, size_t *password_size,
                       zc_device_status_t *status) {
    int err = EXIT_FAILURE;
    struct zc_devio_ctx ioctx;
    unsigned char *p = NULL, *p_end = NULL;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = kGetPassword;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    ioctx.alias_size = (alias_size > sizeof(ioctx.alias) - 1) ? sizeof(ioctx.alias) - 1 : alias_size;
    memcpy(ioctx.alias, alias, ioctx.alias_size);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_GET_PASSWORD, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    if (*status == kNoError) {
        *password_size = ioctx.passwd_size;
        *password = kryptos_newseg(*password_size);
        if (*password != NULL) {
            memcpy(*password, ioctx.passwd, *password_size);
            p = ioctx.passwd;
            p_end = p + *password_size;
            while (p != p_end) {
                *p = 0;
                p++;
            }
        } else {
            err = kGeneralError;
        }
    }

    memset(&ioctx, 0, sizeof(ioctx));
    p = p_end = NULL;

    return err;
}

void zcdev_perror(const zc_device_status_t status) {
    if (status >= kZcDeviceStatusNr) {
        fprintf(stderr, "ERROR: Device status out of range.\n");
        return;
    }
    fprintf(stderr, "ERROR: %s.\n", gZacariasDeviceStatusVerbose[status]);
}

static int zcdev_ioctl(const zc_dev_t zcd, const unsigned long cmd, struct zc_devio_ctx *ioctx) {
#if defined(__unix__)
    int ntry = 10, retval = ioctl(zcd, cmd, ioctx);

    while (retval == -1 && ntry-- > 0) {
        retval = ioctl(zcd, cmd, ioctx);
    }

    if (retval == -1) {
        fprintf(stderr, "ERROR: Unable to communicate with zacarias device : ");
        perror("failure detail : ");
        fprintf(stderr, "\n");
    }

    return retval;
#elif defined(_WIN32)
    int ntry = 10;
    DWORD ret_bytes = 0;
    BOOL done = DeviceIoControl(zcd, (DWORD)cmd, ioctx, sizeof(struct zc_devio_ctx), ioctx, sizeof(struct zc_devio_ctx), &ret_bytes, 0);
    char err_buf[1024];

    memset(err_buf, 0, sizeof(err_buf));

    while (!done && ntry-- > 0) {
        done = DeviceIoControl(zcd, (DWORD)cmd, ioctx, sizeof(struct zc_devio_ctx), ioctx, sizeof(struct zc_devio_ctx), &ret_bytes, 0);
    }

    if (!done) {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       err_buf,
                       sizeof(err_buf),
                       NULL);
        fprintf(stderr, "ERROR: Unable to communicate with zacarias device : failure detail : %s\n", err_buf);
    }

    return (done) ? EXIT_SUCCESS : EXIT_FAILURE;
#else
# error Some code wanted.
#endif
}
