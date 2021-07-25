/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/version.h>
#include <cmd/types.h>
#include <stdlib.h>
#include <stdio.h>

int zc_version(void) {
    fprintf(stdout, "zc version %s\n", ZC_VERSION);
    return EXIT_SUCCESS;
}

int zc_version_help(void) {
    fprintf(stdout, "use: zc version\n");
    return EXIT_SUCCESS;
}
