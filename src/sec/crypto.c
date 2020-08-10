#include <sec/crypto.h>

#define ZACARIAS_PWDB "ZACARIAS PWDB"

static kryptos_u8_t *zacarias_key_crunching(const char *user, const size_t user_size,
                                            const kryptos_u8_t *passwd, const size_t passwd_size, size_t *key_size);

int zacarias_decrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size) {
    int err = 1;
    kryptos_task_ctx t, *ktask = &t;
    kryptos_u8_t *key = NULL;
    size_t key_size;

    kryptos_task_init_as_null(ktask);

    if (profile == NULL || passwd == NULL || passwd_size == 0) {
        goto zacarias_decrypt_pwdb_epilogue;
    }

    if ((*profile)->plbuf != NULL) {
        kryptos_freeseg((*profile)->plbuf, (*profile)->plbuf_size);
        (*profile)->plbuf = NULL;
        (*profile)->plbuf_size = 0;
    }

    key = zacarias_key_crunching((*profile)->user, (*profile)->user_size, passwd, passwd_size, &key_size);

    if (key == NULL) {
        goto zacarias_decrypt_pwdb_epilogue;
    }

    ktask->in = kryptos_pem_get_data(ZACARIAS_PWDB, (*profile)->pwdb, (*profile)->pwdb_size, &ktask->in_size);

    if (ktask->in == NULL) {
        goto zacarias_decrypt_pwdb_epilogue;
    }

    kryptos_task_set_in(ktask, (*profile)->pwdb, (*profile)->pwdb_size);
    kryptos_task_set_decrypt_action(ktask);
    kryptos_run_cipher(aes256, ktask, key, key_size, kKryptosGCM);

    if (kryptos_last_task_succeed(ktask)) {
        err = 0;
        (*profile)->plbuf = ktask->out;
        (*profile)->plbuf_size = ktask->out_size;
    }

zacarias_decrypt_pwdb_epilogue:

    if (key != NULL) {
        kryptos_freeseg(key, key_size);
        key_size = 0;
        key = NULL;
    }

    kryptos_task_free(ktask, KRYPTOS_TASK_IN);

    return err;
}

int zacarias_encrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size) {
    int err = 1;
    kryptos_task_ctx t, *ktask = &t;
    kryptos_u8_t *key = NULL, *old_pwdb = NULL;
    size_t key_size, old_pwdb_size = 0;

    kryptos_task_init_as_null(ktask);

    if (profile == NULL || passwd == NULL || passwd_size == 0) {
        goto zacarias_encrypt_pwdb_epilogue;
    }

    key = zacarias_key_crunching((*profile)->user, (*profile)->user_size, passwd, passwd_size, &key_size);

    if (key == NULL) {
        goto zacarias_encrypt_pwdb_epilogue;
    }

    kryptos_task_set_in(ktask, (*profile)->plbuf, (*profile)->plbuf_size);
    kryptos_task_set_encrypt_action(ktask);
    kryptos_run_cipher(aes256, ktask, key, key_size, kKryptosGCM);

    ktask->in = NULL;
    ktask->in_size = 0;

    if ((*profile)->pwdb != NULL) {
        old_pwdb = (*profile)->pwdb;
        old_pwdb_size = (*profile)->pwdb_size;
        (*profile)->pwdb = NULL;
        (*profile)->pwdb_size = 0;
    }

    if (kryptos_last_task_succeed(ktask)) {
        if (kryptos_pem_put_data(&(*profile)->pwdb,
                                 &(*profile)->pwdb_size,
                                 ZACARIAS_PWDB,
                                 ktask->out,
                                 ktask->out_size) == kKryptosSuccess) {
            err = 0;
        } else {
            (*profile)->pwdb = old_pwdb;
            (*profile)->pwdb_size = old_pwdb_size;
            old_pwdb = NULL;
            old_pwdb_size = 0;
        }
    }

zacarias_encrypt_pwdb_epilogue:

    if (key != NULL) {
        kryptos_freeseg(key, key_size);
        key_size = 0;
        key = NULL;
    }

    if (old_pwdb != NULL) {
        kryptos_freeseg(old_pwdb, old_pwdb_size);
        old_pwdb_size = 0;
        old_pwdb = NULL;
    }

    kryptos_task_free(ktask, KRYPTOS_TASK_OUT);

    return err;

}

static kryptos_u8_t *zacarias_key_crunching(const char *user, const size_t user_size,
                                            const kryptos_u8_t *passwd, const size_t passwd_size, size_t *key_size) {
    kryptos_u32_t parallelism = 1, memory_size_kb = 512, iterations = 128;
    kryptos_u8_t *in = NULL, *key = NULL;
    size_t in_size;

    if (key_size == NULL) {
        return NULL;
    }

    *key_size = 0;

    if (user == NULL || user_size == 0 || passwd == NULL || passwd_size == 0) {
        return NULL;
    }

    in_size = user_size + passwd_size;
    in = (kryptos_u8_t *)kryptos_newseg(in_size);
    if (in == NULL) {
        return NULL;
    }

    memcpy(in, user, user_size);
    memcpy(in + user_size, passwd, passwd_size);

    *key_size = 32;

    key = kryptos_argon2i(in, in_size,
                          (kryptos_u8_t *)user, user_size,
                          parallelism, *key_size, memory_size_kb, iterations,
                          (kryptos_u8_t *)passwd, passwd_size,
                          "zcr", 3);

    if (key == NULL) {
        *key_size = 0;
    }

zacarias_key_crunching_epilogue:

    if (in != NULL) {
        kryptos_freeseg(in, in_size);
        in = NULL;
        in_size = 0;
    }

    return key;
}

#undef ZACARIAS_PWDB
