#include <ctx/ctx.h>
#include <string.h>

// WARN(Rafael): Avoid using str functions from string.h (it can be able to run inside kernel too).

#define zacarias_profile_ctx_new(p) ( (p) = (zacarias_profile_ctx *) kryptos_newseg(sizeof(zacarias_profile_ctx)),\
                                      (p)->user = NULL, (p)->user_size = 0, (p)->pwdb = NULL, (p)->pwdb_size = 0,\
                                      (p)->last = (p)->next = NULL )

static void zacarias_profile_ctx_del(zacarias_profile_ctx *profile);

int zacarias_profiles_ctx_add(zacarias_profiles_ctx **profiles,
                              char *user, const size_t user_size,
                              kryptos_u8_t *pwdb, const size_t pwdb_size) {
    int err = 0;
    zacarias_profile_ctx *new_profile = NULL;

    if (profiles == NULL || user == NULL || user_size == 0 || pwdb == NULL || pwdb_size == 0 ||
        zacarias_profiles_ctx_get(*profiles, user, user_size) != NULL) {
        err = 1;
        goto zacarias_profiles_ctx_add_epilogue;
    }

    zacarias_profile_ctx_new(new_profile);

    if (new_profile == NULL) {
        err = 1;
        goto zacarias_profiles_ctx_add_epilogue;
    }

    new_profile->user = user;
    new_profile->user_size = user_size;
    new_profile->pwdb = pwdb;
    new_profile->pwdb_size = pwdb_size;

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
    zacarias_profile_ctx *profile, *p;
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
            (*profiles)->tail = p->next;
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
    zacarias_profile_ctx *p;

    if (profile == NULL) {
        return;
    }

    for (p = profile; p != NULL; p = p->next) {
        kryptos_freeseg(p->user, p->user_size);
        kryptos_freeseg(p->pwdb, p->pwdb_size);
        p->user_size = p->pwdb_size = 0;
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
