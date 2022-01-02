/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <linux/kernel.h>
#include <linux/version.h>
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
#if defined(set_fs)
    mm_segment_t old_fs = get_fs();
#endif
    int err = 0;
    ssize_t bytes_total = 0;

    if (filepath == NULL || buf == NULL || buf_size == NULL) {
        return -EINVAL;
    }

#if defined(set_fs)
    set_fs(KERNEL_DS);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
    err = vfs_stat(filepath, &ks);
#endif

#if defined(set_fs)
    set_fs(old_fs);
#endif

    if (err != 0) {
        return err;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
    data_size = ks.size;

    data = (char *) kmalloc(data_size, GFP_ATOMIC);
    if (data == NULL) {
        return -EFAULT;
    }
#endif

    file = filp_open(filepath, O_RDONLY, 0);

    if (IS_ERR(file)) {
        return -EFAULT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    data_size = (size_t)i_size_read(file->f_path.dentry->d_inode);
    data = (char *) kmalloc(data_size, GFP_ATOMIC);
    if (data == NULL) {
        return -EFAULT;
    }
#endif

    // INFO(Rafael): Unfortunately, Linux kernel is a mess with all thousand cosmetic changes.
    //               Compare FreeBSD and Windows device drivers versions and you will understand
    //               my complaint. This is messy and unstable for developers. This mess and constant
    //               compatibility breaking is frustrating and pisses me off a bunch. I am tired of
    //               those little problems from a version to another. I am starting to let Linux and
    //               focusing only on Windows and FreeBSD kernels, I want to develop my own stuff without
    //               having to be worried about so basic stuff that should be stable and untouched.
    //               Some points in a software after some years must become axioms, they work, people
    //               have been using it among different versions through years and period. Break your
    //               leg before breaking backward compatibility should be a rule of thumb for some
    //               critical areas on their code base.
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,11,0)
    bytes_total = kernel_read(file, 0, data, data_size);
#else
    bytes_total = kernel_read(file, data, data_size, 0);
#endif
    filp_close(file, NULL);

    *(char **)buf = data;
    *buf_size = data_size;

    return (bytes_total == data_size) ? 0 : 1;
}
