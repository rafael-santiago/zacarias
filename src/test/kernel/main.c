/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <kutest.h>
#include <sec/crypto.h>
#include <sec/plbuf_editor.h>
#include <ctx/ctx.h>
#if defined(__linux__)
# include <string.h>
#endif

KUTE_DECLARE_TEST_CASE(sec_tests);
KUTE_DECLARE_TEST_CASE(crypto_tests);
KUTE_DECLARE_TEST_CASE(plbuf_editor_tests);
KUTE_DECLARE_TEST_CASE(zacarias_gen_userkey_tests);

KUTE_MAIN(sec_tests);

KUTE_TEST_CASE(sec_tests)
    KUTE_RUN_TEST(crypto_tests);
    KUTE_RUN_TEST(plbuf_editor_tests);
    KUTE_RUN_TEST(zacarias_gen_userkey_tests);
KUTE_TEST_CASE_END


KUTE_TEST_CASE(zacarias_gen_userkey_tests)
    struct test_ctx {
        size_t size;
    } test_vector[] = {
        { 1 },
        { 2 },
        { 3 },
        { 4 },
        { 5 },
        { 6 },
        { 7 },
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);
    kryptos_u8_t *key;
    size_t key_size;

    KUTE_ASSERT(zacarias_gen_userkey(NULL) == NULL);
    key_size = 1000;
    KUTE_ASSERT(zacarias_gen_userkey(&key_size) == NULL);
    key_size = 0;
    key = zacarias_gen_userkey(&key_size);
    KUTE_ASSERT(key != NULL);
    kryptos_freeseg(key, key_size);

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
        key_size = test->size;
        key = zacarias_gen_userkey(&key_size);
        KUTE_ASSERT(key_size == test->size);
        KUTE_ASSERT(key != NULL);
        kryptos_freeseg(key, key_size);
        test++;
    }
KUTE_TEST_CASE_END

KUTE_TEST_CASE(plbuf_editor_tests)
    kryptos_u8_t *plbuf = NULL;
    size_t plbuf_size = 0;
    struct test_ctx {
        kryptos_u8_t *alias;
        kryptos_u8_t *passwd;
    } test_vector[] = {
        { (kryptos_u8_t *)"nasa", (kryptos_u8_t *)"12344321" },
        { (kryptos_u8_t *)"cern", (kryptos_u8_t *)"0000000." },
        { (kryptos_u8_t *)"home", (kryptos_u8_t *)"101010-----12939123" },
    }, *test, *test_end;
    kryptos_u8_t *passwd;
    size_t passwd_size;
    kryptos_u8_t *aliases;
    size_t aliases_size;

    test = &test_vector[0];
    test_end = test + sizeof(test_vector) / sizeof(test_vector[0]);

    KUTE_ASSERT(plbuf_edit_add(NULL, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) != 0);
    KUTE_ASSERT(plbuf_edit_add(&plbuf, NULL, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) != 0);
    KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, NULL, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) != 0);
    KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, 0,
                                   test->passwd, strlen((char *)test->passwd)) != 0);
    KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   NULL, strlen((char *)test->passwd)) != 0);
    KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, 0) != 0);

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) == 0);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_find(NULL, plbuf_size, (kryptos_u8_t *)"cern", 3) == 0);
    KUTE_ASSERT(plbuf_edit_find(plbuf, 0, (kryptos_u8_t *)"cern", 3) == 0);
    KUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, NULL, 3) == 0);
    KUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, (kryptos_u8_t *)"cern", 0) == 0);

    KUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, (kryptos_u8_t *)"404", 3) == 0);

    KUTE_ASSERT(plbuf_edit_aliases(NULL, plbuf_size, &aliases_size) == NULL);
    KUTE_ASSERT(plbuf_edit_aliases(plbuf, 0, &aliases_size) == NULL);
    KUTE_ASSERT(plbuf_edit_aliases(plbuf, plbuf_size, NULL) == NULL);

    aliases_size = 0;
    aliases = plbuf_edit_aliases(plbuf, plbuf_size, &aliases_size);

    KUTE_ASSERT(aliases != NULL && aliases_size != 0);

    KUTE_ASSERT(strstr(aliases, "nasa") != NULL);
    KUTE_ASSERT(strstr(aliases, "cern") != NULL);
    KUTE_ASSERT(strstr(aliases, "home") != NULL);
    kryptos_freeseg(aliases, aliases_size);

    test = &test_vector[0];

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, test->alias, strlen((char *)test->alias)) == 1);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"404", 3) != 0);

    test = &test_vector[0];

    KUTE_ASSERT(plbuf_edit_del(NULL, &plbuf_size, test->alias, strlen((char *)test->alias)) != 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, 0, test->alias, strlen((char *)test->alias)) != 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, NULL, strlen((char *)test->alias)) != 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, 0) != 0);

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias)) == 0);
        test++;
    }

    test = &test_vector[0];

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) == 0);
        test++;
    }

    test = test_end - 1;

    while (test != (&test_vector[0] - 1)) {
        KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias)) == 0);
        test--;
    }

    test = &test_vector[0];

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) == 0);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"cern", strlen("cern")) == 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"home", strlen("home")) == 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"nasa", strlen("nasa")) == 0);

    test = &test_vector[0];

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) == 0);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"cern", strlen("cern")) == 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"nasa", strlen("nasa")) == 0);
    KUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, (kryptos_u8_t *)"home", strlen("home")) == 0);

    test = &test_vector[0];

    while (test != test_end) {
        KUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen((char *)test->alias),
                                   test->passwd, strlen((char *)test->passwd)) == 0);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_shuffle(&plbuf, &plbuf_size) == 0);
    KUTE_ASSERT(plbuf_edit_detach(&plbuf, &plbuf_size) == 0);

    test = &test_vector[0];

    KUTE_ASSERT(plbuf_edit_passwd(NULL, plbuf_size, test->alias, strlen((char *)test->alias), &passwd_size) == NULL);
    KUTE_ASSERT(plbuf_edit_passwd(plbuf, 0, test->alias, strlen((char *)test->alias), &passwd_size) == NULL);
    KUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, NULL, strlen((char *)test->alias), &passwd_size) == NULL);
    KUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, test->alias, 0, &passwd_size) == NULL);
    KUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, test->alias, strlen((char *)test->alias), NULL) == NULL);

    while (test != test_end) {
        passwd = plbuf_edit_passwd(plbuf, plbuf_size, test->alias, strlen((char *)test->alias), &passwd_size);
        KUTE_ASSERT(passwd != NULL);
        KUTE_ASSERT(passwd_size == strlen((char *)test->passwd));
        KUTE_ASSERT(memcmp(passwd, test->passwd, passwd_size) == 0);
        kryptos_freeseg(passwd, passwd_size);
        test++;
    }

    KUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, (kryptos_u8_t *)"Greenfuzz", strlen("Greenfuzz"), &passwd_size) == NULL);

    kryptos_freeseg(plbuf, plbuf_size);
KUTE_TEST_CASE_END

KUTE_TEST_CASE(crypto_tests)
    zacarias_profiles_ctx *profiles;
    kryptos_u8_t pwdb[ZC_STR_NR];
    char user[ZC_STR_NR];
    char pwdb_path[ZC_STR_NR];
    size_t pwdb_size;

    zacarias_profiles_ctx_init(profiles);

    KUTE_ASSERT(zacarias_encrypt_pwdb(NULL, (kryptos_u8_t *)"boo", 3) != 0);
    KUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, NULL, 3) != 0);
    KUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 0) != 0);

    KUTE_ASSERT(zacarias_decrypt_pwdb(NULL, (kryptos_u8_t *)"boo", 3) != 0);
    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, NULL, 3) != 0);
    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 0) != 0);

    memset(pwdb, 0, sizeof(pwdb));
    memset(user, 0, sizeof(user));
    memset(pwdb_path, 0, sizeof(pwdb_path));

    memcpy(user, "abc", 3);
    memcpy(pwdb_path, "/k/root", 7);
    pwdb_size = strlen("passwd\tZm9vYmFy\n");
    memcpy(pwdb, "passwd\tZm9vYmFy\n\0", pwdb_size + 1);

    KUTE_ASSERT(zacarias_profiles_ctx_add(&profiles, user, strlen(user), pwdb_path, strlen(pwdb_path), pwdb, strlen((char *)pwdb)) == 0);
    profiles->head->plbuf_size = pwdb_size;
    profiles->head->plbuf = (kryptos_u8_t *) kryptos_newseg(profiles->head->plbuf_size + 1);
    KUTE_ASSERT(profiles->head->plbuf != NULL);
    memcpy(profiles->head->plbuf, pwdb, profiles->head->plbuf_size);

    KUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3) == 0);
    KUTE_ASSERT(profiles->head->plbuf == NULL);
    KUTE_ASSERT(profiles->head->plbuf_size == 0);
    KUTE_ASSERT(profiles->head->pwdb != NULL);
    KUTE_ASSERT(profiles->head->pwdb_size != 0);
    KUTE_ASSERT(memcmp(profiles->head->pwdb, "passwd\tZm9vYmFy\n", pwdb_size) != 0);

    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, (kryptos_u8_t *)"Boo", 3) != 0);
    KUTE_ASSERT(profiles->head->plbuf == NULL);
    KUTE_ASSERT(profiles->head->plbuf_size == 0);
    KUTE_ASSERT(profiles->head->pwdb != NULL);
    KUTE_ASSERT(profiles->head->pwdb_size != 0);

    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3) == 0);
    KUTE_ASSERT(profiles->head->pwdb != NULL);
    KUTE_ASSERT(profiles->head->pwdb_size != 0);
    KUTE_ASSERT(profiles->head->plbuf != NULL);
    KUTE_ASSERT(profiles->head->plbuf_size != 0);
    KUTE_ASSERT(memcmp(profiles->head->plbuf, "passwd\tZm9vYmFy\n", pwdb_size) == 0);

    KUTE_ASSERT(zacarias_setkey_pwdb(NULL, (kryptos_u8_t *)"boo", 3, (kryptos_u8_t *)"foobar", 6) != 0);
    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, NULL, 3, (kryptos_u8_t *)"foobar", 6) != 0);
    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 0, (kryptos_u8_t *)"foobar", 6) != 0);
    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3, NULL, 6) != 0);
    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3, (kryptos_u8_t *)"foobar", 0) != 0);

    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3, (kryptos_u8_t *)"foobar", 6) == 0);
    KUTE_ASSERT(profiles->head->pwdb != NULL);
    KUTE_ASSERT(profiles->head->pwdb_size != 0);
    KUTE_ASSERT(profiles->head->plbuf != NULL);
    KUTE_ASSERT(profiles->head->plbuf_size != 0);
    KUTE_ASSERT(memcmp(profiles->head->plbuf, "passwd\tZm9vYmFy\n", pwdb_size) == 0);

    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, (kryptos_u8_t *)"boo", 3) != 0);

    KUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, (kryptos_u8_t *)"foobar", 6) == 0);
    KUTE_ASSERT(profiles->head->pwdb != NULL);
    KUTE_ASSERT(profiles->head->pwdb_size != 0);
    KUTE_ASSERT(profiles->head->plbuf != NULL);
    KUTE_ASSERT(profiles->head->plbuf_size != 0);
    KUTE_ASSERT(memcmp(profiles->head->plbuf, "passwd\tZm9vYmFy\n", pwdb_size) == 0);

    KUTE_ASSERT(zacarias_setkey_pwdb(&profiles->head, (kryptos_u8_t *)"FOOBAR", 6, (kryptos_u8_t *)"boo", 3) != 0);

    zacarias_profiles_ctx_deinit(profiles);
KUTE_TEST_CASE_END
