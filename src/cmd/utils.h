/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_CMD_UTILS_H
#define ZACARIAS_CMD_UTILS_H 1

#include <stdlib.h>

void del_scr_line(void);

char prompt(const char *question, const char *options, const size_t options_size);

char *get_canonical_path(char *dest, const size_t dest_size, const char *src, const size_t src_size);

#endif