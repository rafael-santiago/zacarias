/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/didumean.h>
#include <cmd/l_distance.h>

void didumean(const char *what,
              char **dest, const size_t dest_nr,
              const char **src, const size_t src_nr, const size_t max_distance) {
    if (what == NULL || dest == NULL || dest_nr == 0 || src == NULL || src_nr == 0) {
        return;
    }

    const char **psrc = src, **psrc_end = src + src_nr;
    char **pdest = dest, **pdest_end = dest + dest_nr;
    int curr_dist;

    *pdest = NULL;

    while (pdest != pdest_end && psrc != psrc_end) {
        curr_dist = l_distance(what, *psrc);
        if (curr_dist >= 1 && curr_dist <= max_distance) {
            *pdest = (char *)*psrc;
            pdest++;
            if (pdest != pdest_end) {
                *pdest = NULL;
            }
        }
        psrc++;
    }
}
