#include <linux/kio_impl.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <kryptos.h>

int kread_pwdb_impl(const char *filepath, unsigned char *password, const size_t password_size, void **buf, size_t *buf_size) {
    return -1;
}

int kwrite_impl(const char *filepath, void *buf, const size_t buf_size) {
    struct file *file;

    if (filepath == NULL || buf == NULL || buf_size == 0) {
        return -EINVAL;
    }

    file = filp_open(filepath, O_WRONLY | O_CREAT, 0664);

    if (file == NULL) {
        return -EFAULT;
    }

    kernel_write(file, buf, buf_size, 0);
    filp_close(file, NULL);

    return 0;
}

int kread_impl(const char *filepath, void **buf, size_t *buf_size) {
    struct kstat ks;
    char *data;
    size_t data_size;
    struct file *file;

    if (filepath == NULL || buf == NULL || buf_size == NULL) {
        return -EINVAL;
    }

    vfs_stat(filepath, &ks);

    data_size = ks.size;

    data = (char *) kmalloc(data_size, GFP_ATOMIC);
    if (data == NULL) {
        return -EFAULT;
    }

    file = filp_open(filepath, O_RDONLY, 0);

    if (file == NULL) {
        return -EFAULT;
    }

    kernel_read(file, 0, data, data_size);
    filp_close(file, NULL);

    *(char **)buf = data;
    *buf_size = data_size;

    return 0;
}
