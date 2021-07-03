/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_CMD_DIDUMEAN_H
#define ZACARIAS_CMD_DIDUMEAN_H 1

#include <stdlib.h>

void didumean(const char *what,
              char **dest, const size_t dest_nr,
              const char **src, const size_t src_nr, const size_t max_distance);

#endif
