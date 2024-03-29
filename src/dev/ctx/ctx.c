/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <ctx/ctx.h>
#ifndef KRYPTOS_KERNEL_MODE
# include <string.h>
#endif
#include <defs/io.h>

// WARN(Rafael): Avoid using str functions from string.h (it need to be able to run inside kernel too).

#define zacarias_profile_ctx_new(p) ( (p) = (zacarias_profile_ctx *) kryptos_newseg(sizeof(zacarias_profile_ctx)),\
                                      memset(p, 0, sizeof(zacarias_profile_ctx)) )

static void zacarias_profile_ctx_del(zacarias_profile_ctx *profile);

int zacarias_profiles_ctx_add(zacarias_profiles_ctx **profiles,
                              char *user, const size_t user_size,
                              char *pwdb_path, const size_t pwdb_path_size,
                              kryptos_u8_t *pwdb, const size_t pwdb_size) {
    int err = 0;
    zacarias_profile_ctx *new_profile = NULL;

    if (profiles == NULL || user == NULL || user_size == 0 || pwdb == NULL || pwdb_size == 0 ||
        pwdb_path == NULL || pwdb_path_size == 0 || zacarias_profiles_ctx_get(*profiles, user, user_size) != NULL) {
        err = 1;
        goto zacarias_profiles_ctx_add_epilogue;
    }

    zacarias_profile_ctx_new(new_profile);

    if (new_profile == NULL) {
        err = 1;
        goto zacarias_profiles_ctx_add_epilogue;
    }

    new_profile->user_size = (user_size < sizeof(new_profile->user) - 1) ? user_size : sizeof(new_profile->user) - 1;
    memcpy(new_profile->user, user, new_profile->user_size);

    new_profile->pwdb_path_size = (pwdb_path_size < sizeof(new_profile->pwdb_path) - 1) ? pwdb_path_size :
                                   sizeof(new_profile->pwdb_path) - 1;
    memcpy(new_profile->pwdb_path, pwdb_path, new_profile->pwdb_path_size);

    new_profile->pwdb = (kryptos_u8_t *) kryptos_newseg(pwdb_size + 1);
    if (new_profile->pwdb == NULL) {
        err = 1;
        goto zacarias_profiles_ctx_add_epilogue;
    }

    memset(new_profile->pwdb, 0, pwdb_size + 1);
    new_profile->pwdb_size = pwdb_size;
    memcpy(new_profile->pwdb, pwdb, new_profile->pwdb_size);

    if ((*profiles)->head == NULL) {
        (*profiles)->head = (*profiles)->tail = new_profile;
    } else {
        new_profile->last = (*profiles)->tail;
        (*profiles)->tail->next = new_profile;
        (*profiles)->tail = new_profile;
    }

zacarias_profiles_ctx_add_epilogue:

    return err;
}

int zacarias_profiles_ctx_del(zacarias_profiles_ctx **profiles,
                              const char *user, const size_t user_size) {
    zacarias_profile_ctx *profile;
    int err = 0;

    if (profiles == NULL || user == NULL || user_size == 0) {
        err = 1;
        goto zacarias_profiles_ctx_del_epilogue;
    }

    profile = zacarias_profiles_ctx_get(*profiles, user, user_size);

    if (profile == NULL) {
        err = 1;
        goto zacarias_profiles_ctx_del_epilogue;
    }

    if (profile == (*profiles)->head) {
        if ((*profiles)->tail == (*profiles)->head) {
            (*profiles)->tail = NULL;
        }
        (*profiles)->head = profile->next;
        profile->last = NULL;
    } else if (profile == (*profiles)->tail) {
        (*profiles)->tail = profile->last;
        profile->last->next = NULL;
    } else {
        profile->last->next = profile->next;
        profile->next->last = profile->last;
    }

    profile->next = NULL;
    zacarias_profile_ctx_del(profile);

zacarias_profiles_ctx_del_epilogue:

    return err;
}

zacarias_profile_ctx *zacarias_profiles_ctx_get(zacarias_profiles_ctx *profiles,
                                                const char *user, const size_t user_size) {
    zacarias_profile_ctx *p;

    if (profiles == NULL || user == NULL || user_size == 0) {
        return NULL;
    }

    for (p = profiles->head; p != NULL; p = p->next) {
        if (p->user_size == user_size && memcmp(p->user, user, user_size) == 0) {
            return p;
        }
    }

    return NULL;
}

static void zacarias_profile_ctx_del(zacarias_profile_ctx *profile) {
    zacarias_profile_ctx *p, *t;

    if (profile == NULL) {
        return;
    }

    for (p = profile; p != NULL; p = t) {
        t = p->next;
        memset(p->user, 0, sizeof(p->user));
        memset(p->pwdb_path, 0, sizeof(p->pwdb_path));
        if (p->pwdb != NULL) {
            kryptos_freeseg(p->pwdb, p->pwdb_size);
        }
        p->user_size = p->pwdb_path_size = p->pwdb_size = 0;
        if (p->plbuf != NULL) {
            kryptos_freeseg(p->plbuf, p->plbuf_size);
            p->plbuf_size = 0;
        }
        kryptos_freeseg(p, sizeof(zacarias_profile_ctx));
    }
}

void zacarias_profiles_ctx_deinit(zacarias_profiles_ctx *profiles) {
    if (profiles == NULL) {
        return;
    }
    zacarias_profile_ctx_del(profiles->head);
    profiles->head = profiles->tail = NULL;
    kryptos_freeseg(profiles, sizeof(zacarias_profiles_ctx));
}

#undef zacarias_profile_ctx_new
