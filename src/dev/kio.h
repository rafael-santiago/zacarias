/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_KIO_H
#define ZACARIAS_DEV_KIO_H 1

int kwrite(const char *filepath, void *buf, const size_t buf_size);

int kread(const char *filepath, void **buf, size_t *buf_size);

#endif
