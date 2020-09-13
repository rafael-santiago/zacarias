#include <sec/crypto.h>

#define ZACARIAS_PWDB "ZACARIAS PWDB"

#define ZACARIAS_MAX_USERKEY_SIZE 32

static kryptos_u8_t *zacarias_key_crunching(const char *user, const size_t user_size,
                                            const kryptos_u8_t *passwd, const size_t passwd_size, size_t *key_size);


int zacarias_setkey_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size,
                         const kryptos_u8_t *new_passwd, const size_t new_passwd_size) {
    int err = 1, do_decrypt = 0;

    if (profile == NULL || passwd == NULL || passwd_size == 0 || new_passwd == NULL || new_passwd_size == 0) {
        goto zacarias_setkey_pwdb_epilogue;
    }

    do_decrypt = ((*profile)->plbuf != NULL);

    // WARN(Rafael): We will always decrypt pwdb, even it being already decrypted.
    if ((err = zacarias_decrypt_pwdb(profile, passwd, passwd_size)) != 0) {
        goto zacarias_setkey_pwdb_epilogue;
    }

    if ((err = zacarias_encrypt_pwdb(profile, new_passwd, new_passwd_size)) != 0) {
        goto zacarias_setkey_pwdb_epilogue;
    }

    if (do_decrypt) {
        err = zacarias_decrypt_pwdb(profile, new_passwd, new_passwd_size);
    }

zacarias_setkey_pwdb_epilogue:

    do_decrypt = 0;

    return err;
}

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

    kryptos_task_set_decrypt_action(ktask);
    kryptos_run_cipher(aes256, ktask, key, key_size, kKryptosGCM);

    if (kryptos_last_task_succeed(ktask)) {
        err = 0;
        (*profile)->plbuf = ktask->out;
        (*profile)->plbuf_size = ktask->out_size;
        ktask->out = NULL;
        ktask->out_size = 0;
    }

zacarias_decrypt_pwdb_epilogue:

    kryptos_task_free(ktask, KRYPTOS_TASK_IN | KRYPTOS_TASK_KEY | KRYPTOS_TASK_IV | KRYPTOS_TASK_OUT);

    return err;
}

int zacarias_encrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size) {
    int err = 1;
    kryptos_task_ctx t, *ktask = &t;
    kryptos_u8_t *key = NULL, *old_pwdb = NULL;
    size_t key_size = 0, old_pwdb_size = 0;

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
    } else {
        if ((*profile)->pwdb_size != 0) {
            (*profile)->pwdb_size = 0;
        }
    }

    if (kryptos_last_task_succeed(ktask)) {
        if (kryptos_pem_put_data(&(*profile)->pwdb,
                                 &(*profile)->pwdb_size,
                                 ZACARIAS_PWDB,
                                 ktask->out,
                                 ktask->out_size) == kKryptosSuccess) {
            err = 0;
            kryptos_freeseg((*profile)->plbuf, (*profile)->plbuf_size);
            (*profile)->plbuf = NULL;
            (*profile)->plbuf_size = 0;
        } else {
            (*profile)->pwdb = old_pwdb;
            (*profile)->pwdb_size = old_pwdb_size;
            old_pwdb = NULL;
            old_pwdb_size = 0;
        }
    }

zacarias_encrypt_pwdb_epilogue:

    if (old_pwdb != NULL) {
        kryptos_freeseg(old_pwdb, old_pwdb_size);
        old_pwdb_size = 0;
        old_pwdb = NULL;
    }

    kryptos_task_free(ktask, KRYPTOS_TASK_OUT | KRYPTOS_TASK_KEY | KRYPTOS_TASK_IV);

    key = NULL;
    key_size = 0;

    return err;

}

static kryptos_u8_t *zacarias_key_crunching(const char *user, const size_t user_size,
                                            const kryptos_u8_t *passwd, const size_t passwd_size, size_t *key_size) {
    kryptos_u32_t parallelism = 1, memory_size_kb = 512, iterations = 4;
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

    if (in != NULL) {
        kryptos_freeseg(in, in_size);
        in = NULL;
        in_size = 0;
    }

    return key;
}

unsigned int unbiased_rand_mod(const unsigned n) {
    unsigned int r = 0;

    do {
        do {
            r = kryptos_get_random_byte() << 24 |
                kryptos_get_random_byte() << 16 |
                kryptos_get_random_byte() <<  8 |
                kryptos_get_random_byte();
        } while (r >= 0xFFFFFFFF - (0xFFFFFFFF % n));
        r = r % n;
    } while (r == 0);

    return r;
}

kryptos_u8_t *zacarias_gen_userkey(size_t *size) {
    kryptos_u8_t *key = NULL, *kp, *kp_end;
    static kryptos_u8_t gZacariasUserKeyCharset[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '"', '\'', '!', '@', '#', '$', '%',
        '&', '*', '(', ')', '-', '_', '=', '+', '`', '{', '[', '^', '~', '}', ']', ',', '<', '>', '.', ':', ';', '?', '/',
        '\\', '|'
    };
    static size_t gZacariasUserKeyCharsetNr = sizeof(gZacariasUserKeyCharset) / sizeof(gZacariasUserKeyCharset[0]);

    if (size == NULL) {
        return NULL;
    }

    if (*size == 0) {
        *size = unbiased_rand_mod(ZACARIAS_MAX_USERKEY_SIZE);
    } else if (*size > ZACARIAS_MAX_USERKEY_SIZE) {
        return NULL;
    }

    key = (kryptos_u8_t *) kryptos_newseg(*size);
    if (key == NULL) {
        return NULL;
    }

    kp = key;
    kp_end = kp + *size;

    while (kp != kp_end) {
        *kp = gZacariasUserKeyCharset[unbiased_rand_mod(gZacariasUserKeyCharsetNr)];
        kp++;
    }

    kp = kp_end = NULL;

    return key;
}

#undef ZACARIAS_PWDB
#undef ZACARIAS_MAX_USERKEY_SIZE
