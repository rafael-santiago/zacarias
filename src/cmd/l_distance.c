/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/l_distance.h>
#include <kryptos_memory.h>
#include <string.h>

int l_distance(const char *yterm, const char *xterm) {
    int **distances = NULL;
    int final_distance = 0xFFFFFF;
    size_t yterm_size, xterm_size, y, x;
    int edit_op;

    if (yterm == NULL || xterm == NULL) {
        goto l_distance_epilogue;
    }

    if ((yterm_size = strlen(yterm)) == 0 || (xterm_size = strlen(xterm)) == 0) {
        goto l_distance_epilogue;
    }

    distances = (int **) kryptos_newseg((yterm_size + 1) * sizeof(int *));

    if (distances == NULL) {
        goto l_distance_epilogue;
    }

    for (y = 0; y <= yterm_size; y++) {
        distances[y] = (int *) kryptos_newseg((xterm_size + 1) * sizeof(int));
        if (distances[y] == NULL) {
            goto l_distance_epilogue;
        }
    }

    for (x = 0; x < xterm_size; x++) {
        distances[0][x] = x;
    }

    for (y = 0; y < yterm_size; y++) {
        distances[y][0] = y;
    }

#define levenshtein_min(di_y, di_x, di_yx) ( ((di_x) < (di_y) && (di_x) < (di_yx)) ? (di_x) :\
                                             ((di_y) < (di_x) && (di_y) < (di_yx)) ? (di_y) : (di_yx) )

    for (y = 0; y < yterm_size; y++) {
        for (x = 0; x < xterm_size; x++) {
            edit_op = (yterm[y] == xterm[x]) ? 0 : 2;
            distances[y + 1][x + 1] = levenshtein_min(distances[y][x + 1] + 1,
                                                      distances[y + 1][x] + 1,
                                                      distances[y][x] + edit_op);
        }
    }

#undef levenshtein_min

    final_distance = distances[yterm_size - 1][xterm_size - 1];

l_distance_epilogue:

    if (distances != NULL) {
        for (y = 0; y <= yterm_size; y++) {
            if (distances[y] != NULL) {
                kryptos_freeseg(distances[y], sizeof(int) + xterm_size + 1);
            }
        }
        kryptos_freeseg(distances, 0);
    }

    return final_distance;
}
