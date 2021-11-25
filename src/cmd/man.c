/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/man.h>
#include <cmd/utils.h>
#include <kryptos_memory.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

static char *get_manual_data(size_t *buf_size);

int zc_man(void) {
    int err = EXIT_FAILURE;
    size_t man_buf_size;
    char *man_buf = get_manual_data(&man_buf_size);
    FILE *zc_out = NULL;
    if (man_buf == NULL) {
        fprintf(stderr, "ERROR: Unable to get manual data. Try to reinstall Zacarias, your copy seems screwed.\n");
        return EXIT_FAILURE;
    }

    if ((zc_out = get_stdout()) != NULL) {
        fwrite(man_buf, 1, man_buf_size, zc_out);
        err = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "ERROR: Unable to get the standard output.\n");
    }

    if (zc_out != NULL && zc_out != stdout) {
        pclose(zc_out);
    }

    if (man_buf != NULL) {
        kryptos_freeseg(man_buf, man_buf_size);
    }

    return err;
}

int zc_man_help(void) {
    fprintf(stdout, "use: zc man\n");
    return EXIT_SUCCESS;
}

static char *get_manual_data(size_t *buf_size) {
    char *buf = NULL;
    struct stat st;
    FILE *fp = NULL;
#if defined(__unix__)
    const char *manual_path = "/usr/local/share/zacarias/doc/MANUAL.txt";
#elif defined(_WIN32)
    const char *manual_path = "C:\\zacarias\\doc\\MANUAL.txt";
#else
# error Some code wanted.
#endif

    if (buf_size == NULL) {
        return NULL;
    }

    if (stat(manual_path, &st) != EXIT_SUCCESS) {
        return NULL;
    }

    *buf_size = st.st_size;
    if ((buf = (char *)kryptos_newseg(*buf_size)) == NULL) {
        *buf_size = 0;
        return NULL;
    }

    if ((fp = fopen(manual_path, "rb")) == NULL) {
        kryptos_freeseg(buf, *buf_size);
        *buf_size = 0;
        return NULL;
    }

    fread(buf, 1, *buf_size, fp);

    fclose(fp);

    return buf;
}
