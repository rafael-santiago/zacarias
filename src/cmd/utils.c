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
#if defined(_WIN32)
# include <windows.h>
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
    memset(dest, 0, dest_size);
    if (GetFullPathNameA(src, dest_size, dest, NULL) == 0) {
        return NULL;
    }
#else
# error Some code wanted.
#endif

    return dest;
}

#if defined(_WIN32)
char *get_ntpath(char *dest, const size_t dest_size, const char *src, const size_t src_size) {
    char dos_device[MAX_PATH];
    char *s = NULL, *s_end = NULL;
    char device_name[MAX_PATH];
    char temp[4096];
    DWORD device_name_size;
    size_t temp_size;

    if (dest == NULL || dest_size == 0 || src == NULL || src_size == 0) {
        return NULL;
    }

    if ((s = strstr(src, ":")) == NULL) {
        return NULL;
    }

    s_end = s + 1;

    if ((s_end - src) > sizeof(dos_device) - 1) {
        return NULL;
    }

    memset(dos_device, 0, sizeof(dos_device));
    memcpy(dos_device, src, s_end - src);
    device_name_size = QueryDosDeviceA(dos_device, device_name, sizeof(device_name) - 1); 

    if (device_name_size == 0) {
        return NULL;
    }

    memset(temp, 0, sizeof(temp));
    temp_size = snprintf(temp, sizeof(temp) - 1, "%s\\%s", device_name, s_end + ((size_t)s_end[0] == '\\' && s_end < (src + src_size)));

    if (temp_size > (dest_size - 1)) {
        return NULL;
    }

    snprintf(dest, dest_size - 1, "%s", temp);

    return dest;
}
#endif
