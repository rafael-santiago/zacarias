#include <cmd/devio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

static int zcdev_ioctl(const int zcd, const int cmd, struct zc_devio_ctx *ioctx);

int zcdev_open(void) {
    int zcd = open(CDEVNAME, O_RDWR);
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
                 zc_device_status_t *status) {
    struct zc_devio_ctx ioctx;
    int err = 0;

    memset(&ioctx, 0, sizeof(ioctx));

    ioctx.action = kAttachProfile;
    ioctx.pwdb_path = (char *)pwdb_path;
    ioctx.pwdb_path_size = pwdb_path_size;
    ioctx.user = (char *)user;
    ioctx.user_size = user_size;
    ioctx.pwdb_passwd = (unsigned char *)pwdb_passwd;
    ioctx.pwdb_passwd_size = pwdb_passwd_size;
    ioctx.session_passwd = (unsigned char *)session_passwd;
    ioctx.session_passwd_size = session_passwd_size;
    ioctx.sessioned = (session_passwd == NULL) ? 0 : 1;

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
    ioctx.user = (char *) user;
    ioctx.user_size = user_size;
    ioctx.pwdb_passwd = (unsigned char *) pwdb_passwd;
    ioctx.pwdb_passwd_size = pwdb_passwd_size;

    if ((err = zcdev_ioctl(zcd, ZACARIAS_DETACH_PROFILE, &ioctx)) == 0) {
        *status = ioctx.status;
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
    int ntry = 10, retval = ioctl(zcd, cmd, &ioctx);

    while (retval == -1 && ntry-- > 0) {
        retval = ioctl(zcd, cmd, &ioctx);
    }

    if (retval == -1) {
        fprintf(stderr, "ERROR: Unable to communicate with zacarias device : ");
        perror("failure detail : ");
        fprintf(stderr, "\n");
    }

    return retval;
}
