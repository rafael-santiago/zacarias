/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/syscallsubr.h>
#include <sys/proc.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <kio.h>

int kwrite(const char *filepath, void *buf, const size_t buf_size) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
    struct uio uio_ctx = { 0 };
    struct iovec iovec_ctx = { 0 };
    struct thread *td = curthread;
    int fd = -1;
    int bc = -1;
    if (kern_openat(td,
                    AT_FDCWD,
                    filepath,
                    UIO_SYSSPACE,
                    O_WRONLY | O_CREAT | O_TRUNC, 0664) > -1) {
        fd = td->td_retval[0];
        iovec_ctx.iov_base = buf;
        iovec_ctx.iov_len = buf_size;
        uio_ctx.uio_td = td;
        uio_ctx.uio_rw = UIO_WRITE;
        uio_ctx.uio_iov = &iovec_ctx;
        uio_ctx.uio_iovcnt = 1;
        uio_ctx.uio_segflg = UIO_SYSSPACE;
        uio_ctx.uio_offset = 0;
        uio_ctx.uio_resid = iovec_ctx.iov_len;
        bc = kern_writev(td, fd, &uio_ctx);
        kern_close(td, fd);
    }

    return bc;
#pragma pop
#pragma pop
}

int kread(const char *filepath, void **buf, size_t *buf_size) {
    struct uio uio_ctx = { 0 };
    struct iovec iovec_ctx = { 0 };
    struct thread *td = curthread;
    int fd = -1;
    struct stat st = { 0 };

    if (filepath == NULL || buf == NULL || buf_size == NULL) {
        return 1;
    }

    *buf_size = 0;

    if (kern_openat(td, AT_FDCWD, filepath, UIO_SYSSPACE, O_RDONLY, 0664) > -1) {
        fd = td->td_retval[0];
        if (kern_fstat(td, fd, &st) != 0) {
            goto kread_impl_epilogue;
        }

        *buf_size = st.st_size;
        *buf = (char *) malloc(*buf_size + 1, M_TEMP, M_NOWAIT);
        if (*buf == NULL) {
            goto kread_impl_epilogue;
        }

        memset(*buf, 0, *buf_size + 1);
        iovec_ctx.iov_base = *buf;
        iovec_ctx.iov_len = *buf_size;
        uio_ctx.uio_td = td;
        uio_ctx.uio_rw = UIO_READ;
        uio_ctx.uio_iov = &iovec_ctx;
        uio_ctx.uio_iovcnt = 1;
        uio_ctx.uio_segflg = UIO_SYSSPACE;
        uio_ctx.uio_offset = 0;
        uio_ctx.uio_resid = iovec_ctx.iov_len;
        if (kern_readv(td, fd, &uio_ctx) != 0) {
            *buf_size = 0;
            free(*buf, M_TEMP);
            *buf = NULL;
        }
    }

kread_impl_epilogue:

    if (fd != -1) {
        kern_close(td, fd);
    }

    return (*buf_size > 0) ? 0 : 1;
}

