/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_LINUX_KIO_IMPL_H
#define ZACARIAS_DEV_LINUX_KIO_IMPL_H 1

#include <linux/kernel.h>

int kwrite_impl(const char *filepath, void *buf, const size_t buf_size);

int kread_impl(const char *filepath, void **buf, size_t *buf_size);

int kread_pwdb_impl(const char *filepath, unsigned char *password, const size_t password_size, void **buf, size_t *buf_size);

#endif