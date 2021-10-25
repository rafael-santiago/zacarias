/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/utils.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#if defined(_WIN32
# include <shlwapi.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void del_scr_line(void) {
    fprintf(stdout, "\r                                                                                              \r");
    fflush(stdout);
}

char prompt(const char *question, const char *options, const size_t options_size) {
    char opt[2];

    opt[0] = opt[1] = 0;

    if (question == NULL || options == NULL || options_size == 0) {
        return 0;
    }

    do {
        fprintf(stdout, "\r%s", question);
        fscanf(stdin, "%c%c", &opt[0], &opt[1]);
        opt[1] = 0;
    } while(strstr(options, opt) == NULL);

    return opt[0];
}

char *get_canonical_path(char *dest, const size_t dest_size, const char *src, const size_t src_size) {
#if defined(__unix__)
    char *dest_p, *dest_p_end;
    const char *src_p, *src_p_end;
    char in[4096];
    char out[4096], *out_p;
    struct stat st;

    if (dest == NULL || dest_size == 0 || src == NULL || src_size == 0) {
        return NULL;
    }

    dest_p = dest;
    memset(dest, 0, dest_size);

    if (strstr(src, "/") == NULL) {
        if (getcwd(dest, dest_size - 1) == NULL) {
            return NULL;
        }
        dest_p_end = dest + strlen(dest);
        snprintf(dest_p_end, dest_size - (dest_p_end - dest) - 1, "/%s", src);
        return dest;
    }

    dest_p_end = dest + dest_size;

    src_p = src;
    src_p_end = src_p + src_size;

    out_p = &out[0];

    do {
        if (out_p == NULL) {
            do {
                src_p_end -= 1;
            } while (src_p_end != src && *src_p_end != '/');
        }
        memset(in, 0, sizeof(in));
        memcpy(in, src_p, src_p_end - src_p);
    } while ((out_p = realpath(in, out)) == NULL && src_p_end > src);

    if (src_p_end <= src) {
        return NULL;
    }

    snprintf(dest_p, dest_size - 1, "%s", out);

    if (src_p_end != (src + src_size)) {
        dest_p += strlen(dest_p);
        if (dest_p >= dest_p_end || src_p_end >= (src + src_size)) {
            memset(dest, 0, dest_size);
            return NULL;
        }
        snprintf(dest_p, dest_size - (dest_p_end - dest_p) - 1, "%s", src_p_end);
    }

    while (dest_p_end != dest_p && *dest_p_end != '/') {
        dest_p_end -= 1;
    }

    if (dest_p_end != dest_p) {
        memset(out, 0, sizeof(out));
        memcpy(out, dest_p, dest_p_end - dest_p);
        if (stat(out, &st) != EXIT_SUCCESS) {
            memset(dest, 0, dest_size);
            return NULL;
        }
    }
#elif defined(_WIN32)
    // WARN(Rafael): Some MinGW versions seems not to support ´libpathcch.lib´ by now let's use ´shlwpai.lib´.
    char drive[MAX_PATH] = "", *d = NULL;
    char canonical[MAX_PATH] = "", *c = NULL;
    if (GetCurrentDirectoryA(sizeof(drive), drive) == 0) {
        return NULL;
    }

    if ((d = strstr(drive, ":")) != NULL && (d + 1) < (&drive[0] + sizeof(drive))) {
        d[1] = 0;
    } else {
        return NULL;
    }

    snprintf(dest, dest_size, "%s", drive);
    d = dest + strlen(dest);

    PathCanonicalizeA(canonical, src);
    c = strstr(canonical, ":\\");
    if (canonical[0] == '\\' && strlen(canonical) == 1) {
        canonical[0] = 0;
    } else if (canonical[0] == '\\') {
        c = canonical[1];
    }

    snprintf(d, dest_size - (d - dest), ":\\%s", (c == NULL) ? canonical : c);
#else
# error Some code wanted.
#endif

    return dest;
}