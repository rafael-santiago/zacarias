#include <linux/cdev_init.h>
#include <defs/types.h>
#include <linux/device.h>
#include <linux/fs.h>

#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

static struct file_operations fops = {
    .owner = THIS_MODULE
};

int cdev_init(void) {
    cdev_mtx_init(&g_cdev()->lock);

    g_cdev()->profiles = NULL;

    g_cdev()->major_nr = register_chrdev(0, CDEVNAME, &fops);

    if (g_cdev()->major_nr < 0) {
        printk(KERN_INFO "/dev/zacarias: Error during cdev registration.\n");
        return g_cdev()->major_nr;
    }

    g_cdev()->device_class = class_create(THIS_MODULE, CDEVCLASS);

    if (IS_ERR(g_cdev()->device_class)) {
        unregister_chrdev(g_cdev()->major_nr, CDEVNAME);
        printk(KERN_INFO "/dev/zacarias: Class creation has failed.\n");
        return PTR_ERR(g_cdev()->device_class);
    }

    g_cdev()->device = device_create(g_cdev()->device_class, NULL, MKDEV(g_cdev()->major_nr, 0), NULL, CDEVNAME);

    if (IS_ERR(g_cdev()->device)) {
        class_destroy(g_cdev()->device_class);
        unregister_chrdev(g_cdev()->major_nr, CDEVNAME);
        printk(KERN_INFO "/dev/zacarias: Device file creation failure.\n");
        return PTR_ERR(g_cdev()->device);
    }

    return 0;
}
