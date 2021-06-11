#include <cmd/devio.h>
#include <cmd/utils.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

static int zcdev_ioctl(const int zcd, const int cmd, struct zc_devio_ctx *ioctx);

int zcdev_open(void) {
    int zcd = open("/dev/"CDEVNAME, O_RDWR);
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
}

int zcdev_attach(const int zcd,
                 const char *pwdb_path, const size_t pwdb_path_size,
                 const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 const unsigned char *session_passwd, const size_t session_passwd_size,
                 const int init, zc_device_status_t *status) {
    struct zc_devio_ctx ioctx;
    int err = 0;

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

int zcdev_detach(const int zcd, const char *user, const size_t user_size,
                 const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                 zc_device_status_t *status) {
    struct zc_devio_ctx ioctx;
    int err;

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

int zcdev_add_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const char *alias, const size_t alias_size,
                       const unsigned char *password, const size_t password_size,
                       zc_device_status_t *status) {
    int err;
    struct zc_devio_ctx ioctx;

    ioctx.action = kAddPassword;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

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

int zcdev_del_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const char *alias, const size_t alias_size,
                       zc_device_status_t *status) {
    int err;
    struct zc_devio_ctx ioctx;

    ioctx.action = kDelPassword;

    ioctx.user_size = (user_size > sizeof(ioctx.user) - 1) ? sizeof(ioctx.user) - 1 : user_size;
    memcpy(ioctx.user, user, ioctx.user_size);

    ioctx.pwdb_passwd_size = (pwdb_passwd_size > sizeof(ioctx.pwdb_passwd) - 1) ? sizeof(ioctx.pwdb_passwd) - 1
                                                                                : pwdb_passwd_size;
    memcpy(ioctx.pwdb_passwd, pwdb_passwd, ioctx.pwdb_passwd_size);

    ioctx.alias_size = (alias_size > sizeof(ioctx.alias) - 1) ? sizeof(ioctx.alias) - 1 : alias_size;
    memcpy(ioctx.alias, alias, ioctx.alias_size);

    if ((err = zcdev_ioctl(zcd, ZACARIAS_DEL_PASSWORD, &ioctx)) == 0) {
        *status = ioctx.status;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

int zcdev_get_password(const int zcd, const char *user, const size_t user_size,
                       const unsigned char *pwdb_passwd, const size_t pwdb_passwd_size,
                       const char *alias, const size_t alias_size,
                       unsigned char **password, size_t *password_size,
                       zc_device_status_t *status) {
    int err;
    struct zc_devio_ctx ioctx;

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
        *password = ioctx.passwd;
        *password_size = ioctx.passwd_size;
    }

    memset(&ioctx, 0, sizeof(ioctx));

    return err;
}

void zcdev_perror(const zc_device_status_t status) {
    if (status >= kZcDeviceStatusNr) {
        fprintf(stderr, "ERROR: Device status out of range.\n");
        return;
    }
    fprintf(stderr, "ERROR: %s.\n", gZacariasDeviceStatusVerbose[status]);
}

static int zcdev_ioctl(const int zcd, const int cmd, struct zc_devio_ctx *ioctx) {
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
}
