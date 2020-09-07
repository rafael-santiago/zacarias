#ifndef ZACARIAS_DEV_DEFS_IO_H
#define ZACARIAS_DEV_DEFS_IO_H 1

#if defined(__linux__)
# include <linux/ioctl.h>
# include <linux/kernel.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
# include <sys/ioccom.h>
#else
# error Some code wanted.
#endif

typedef enum {
    kAttachProfile   = 0x00000001,
    kWithSessionKey  = 0x00000002,
    kDetachProfile   = 0x00000004,
    kAddPassword     = 0x00000008,
    kDelPassword     = 0x00000010,
    kGetPassword     = 0x00000020,
    kWithRndPassword = 0x00000040,
}zc_device_action_t;

typedef enum {
    kNoError = 0,
    kInvalidParams,
    kAliasNotFound,
    kProfileNotFound,
    kProfilePreviouslyAttached,
    kProfileNotAttached,
    kAuthenticationFailure,
    kGeneralError,
    kPWDBReadingError,
    kPWDBWritingError,
    kUnknownDeviceCommand,
    kZcDeviceStatusNr,
}zc_device_status_t;

static char *gZacariasDeviceStatusVerbose[kZcDeviceStatusNr] = {
    "No error",
    "Invalid parameters",
    "Alias not found",
    "Profile not found",
    "Profile previously attached",
    "Profile not attached",
    "Authentication failure",
    "General error",
    "Unknown device command",
};

#define zc_dev_decode_status(s) ( ((s) >= 0 && (s) < kZcDeviceStatusNr) ?\
                                        gZacariasDeviceStatusVerbos[(s)] : "Bad device status provided" )

struct zc_devio_ctx {
    zc_device_action_t action;
    zc_device_status_t status;
    char *pwdb_path;
    size_t pwdb_path_size;
    char *user;
    size_t user_size;
    unsigned char *session_passwd;
    size_t session_passwd_size;
    unsigned char *pwdb_passwd;
    size_t pwdb_passwd_size;
    unsigned char *passwd;
    size_t passwd_size;
    char *alias;
    size_t alias_size;
};

#define ZACARIAS_IOC_MAGIC 'Z'

#define ZACARIAS_ATTACH_PROFILE _IOWR(ZACARIAS_IOC_MAGIC, 0, struct zc_devio_ctx *)
#define ZACARIAS_DETACH_PROFILE _IOWR(ZACARIAS_IOC_MAGIC, 1, struct zc_devio_ctx *)
#define ZACARIAS_ADD_PASSWORD   _IOWR(ZACARIAS_IOC_MAGIC, 2, struct zc_devio_ctx *)
#define ZACARIAS_DEL_PASSWORD   _IOWR(ZACARIAS_IOC_MAGIC, 3, struct zc_devio_ctx *)
#define ZACARIAS_GET_PASSWORD   _IOWR(ZACARIAS_IOC_MAGIC, 4, struct zc_devio_ctx *)

#endif
