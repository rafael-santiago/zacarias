#include <linux/cdev_ioctl.h>
#include <defs/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <actions.h>

long cdev_ioctl(struct file *fp, unsigned int cmd, unsigned long user_param) {
    long error = 0;
    struct zc_devio_ctx devio, *dev_p;

    if ((void *)user_param == NULL ||
        !access_ok(VERIFY_WRITE, (void __user *)user_param, _IOC_SIZE(cmd))) {
        // INFO(Rafael): Verifying for writing includes verifying for reading.
        error = EFAULT;
        goto cdev_ioctl_epilogue;
    }

    if (kcpy(&devio, (struct zc_devio_ctx *)user_param, sizeof(struct zc_devio_ctx)) != 0) {
        error = EFAULT;
        goto cdev_ioctl_epilogue;
    }

    dev_p = &devio;

    switch (cmd) {
        case ZACARIAS_ATTACH_PROFILE:
            printk(KERN_INFO "-- ZACARIAS_ATTACH_PROFILE IOCTL\n");
            error = zc_dev_act_attach_profile(&dev_p);
            break;

        case ZACARIAS_DETACH_PROFILE:
            error = zc_dev_act_detach_profile(&dev_p);
            break;

        case ZACARIAS_ADD_PASSWORD:
            error = zc_dev_act_add_password(&dev_p);
            break;

        case ZACARIAS_DEL_PASSWORD:
            error = zc_dev_act_del_password(&dev_p);
            break;

        case ZACARIAS_GET_PASSWORD:
            error = zc_dev_act_get_password(&dev_p);
            break;

        case ZACARIAS_IS_SESSIONED_PROFILE:
            error = zc_dev_act_is_sessioned_profile(&dev_p);
            break;

        case ZACARIAS_SETKEY:
            error = zc_dev_act_setkey(&dev_p);
            break;

        default:
            devio.status = kUnknownDeviceCommand;
            break;
    }

    if (ucpy((void __user *)user_param, &devio, sizeof(struct zc_devio_ctx)) != 0) {
        error = EFAULT;
    }

cdev_ioctl_epilogue:

    return (error == 0) ? error : -error;
}
