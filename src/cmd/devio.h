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
                 zc_device_status_t *status);

void zcdev_perror(const zc_device_status_t status);

#endif
