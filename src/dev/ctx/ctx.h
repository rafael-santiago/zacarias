/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_CTX_CTX_H
#define ZACARIAS_CTX_CTX_H 1

#include <defs/io.h>
#include <kryptos.h>

#define zacarias_profiles_ctx_init(p) ( (p) = (zacarias_profiles_ctx *) kryptos_newseg(sizeof(zacarias_profiles_ctx)),\
                                        (p)->head = (p)->tail = NULL )

typedef struct zacarias_profile {
    char user[ZC_STR_NR];
    size_t user_size;
    char pwdb_path[ZC_STR_NR];
    size_t pwdb_path_size;
    kryptos_u8_t *pwdb;
    size_t pwdb_size;
    kryptos_u8_t *plbuf;
    size_t plbuf_size;
    unsigned char sessioned;
    struct zacarias_profile *next, *last;
}zacarias_profile_ctx;

typedef struct {
    zacarias_profile_ctx *head, *tail;
}zacarias_profiles_ctx;

int zacarias_profiles_ctx_add(zacarias_profiles_ctx **profiles,
                              char *user, const size_t user_size,
                              char *pwdb_path, const size_t pwdb_path_size,
                              kryptos_u8_t *pwdb, const size_t pwdb_size);

int zacarias_profiles_ctx_del(zacarias_profiles_ctx **profiles,
                              const char *user, const size_t user_size);

zacarias_profile_ctx *zacarias_profiles_ctx_get(zacarias_profiles_ctx *profiles,
                                                const char *user, const size_t user_size);

void zacarias_profiles_ctx_deinit(zacarias_profiles_ctx *profiles);

#endif
