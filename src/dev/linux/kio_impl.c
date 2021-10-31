/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <kryptos.h>

int kwrite(const char *filepath, void *buf, const size_t buf_size) {
    struct file *file;
    ssize_t res;

    if (filepath == NULL || buf == NULL || buf_size == 0) {
        return -EINVAL;
    }

    file = filp_open(filepath, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, 0664);

    if (file == NULL) {
        return -EFAULT;
    }

    res = kernel_write(file, buf, buf_size, NULL);
    filp_close(file, NULL);
    //printk(KERN_INFO "res: %d == %d\n", res, buf_size);
    return (res == buf_size) ? 0 : 1;
}

int kread(const char *filepath, void **buf, size_t *buf_size) {
    struct kstat ks = { 0 };
    char *data;
    size_t data_size;
    struct file *file;
    mm_segment_t old_fs = get_fs();
    int err = 0;
    ssize_t bytes_total = 0;

    if (filepath == NULL || buf == NULL || buf_size == NULL) {
        return -EINVAL;
    }

    set_fs(KERNEL_DS);
    err = vfs_stat(filepath, &ks);
    set_fs(old_fs);

    if (err != 0) {
        return err;
    }

    data_size = ks.size;

    data = (char *) kmalloc(data_size, GFP_ATOMIC);
    if (data == NULL) {
        return -EFAULT;
    }

    file = filp_open(filepath, O_RDONLY, 0);

    if (file == NULL) {
        return -EFAULT;
    }

    bytes_total = kernel_read(file, 0, data, data_size);
    filp_close(file, NULL);

    *(char **)buf = data;
    *buf_size = data_size;

    return (bytes_total == data_size) ? 0 : 1;
}
