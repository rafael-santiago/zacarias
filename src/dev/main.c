#define ZACARIAS_DEV_VERSION "0.0.1"

#if defined(__linux__)

#include <linux/cdev_init.h>
#include <linux/cdev_deinit.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Santiago");
MODULE_DESCRIPTION("Zacarias password manager char device");
MODULE_VERSION(ZACARIAS_DEV_VERSION);

static int __init ini(void) {
    return cdev_init();
}

static void __exit finis(void) {
    cdev_deinit();
}

module_init(ini);
module_exit(finis);

#elif defined(__FreeBSD__)

#elif defined(__NetBSD__)

#else
# error Some code wanted
#endif
