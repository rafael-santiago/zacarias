#include <cutest.h>
#include <sec/crypto.h>
#include <sec/plbuf_editor.h>
#include <ctx/ctx.h>
#include <string.h>

CUTE_DECLARE_TEST_CASE(sec_tests);
CUTE_DECLARE_TEST_CASE(crypto_tests);
CUTE_DECLARE_TEST_CASE(plbuf_editor_tests);

CUTE_MAIN(sec_tests);

CUTE_TEST_CASE(sec_tests)
    CUTE_RUN_TEST(crypto_tests);
    CUTE_RUN_TEST(plbuf_editor_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(plbuf_editor_tests)
    kryptos_u8_t *plbuf = NULL;
    size_t plbuf_size = 0;
    struct test_ctx {
        kryptos_u8_t *alias;
        kryptos_u8_t *passwd;
    } test_vector[] = {
        { "nasa", "12344321" },
        { "cern", "0000000." },
        { "home", "101010-----12939123" },
    }, *test, *test_end;
    kryptos_u8_t *passwd;
    size_t passwd_size;

    test = &test_vector[0];
    test_end = test + sizeof(test_vector) / sizeof(test_vector[0]);

    CUTE_ASSERT(plbuf_edit_add(NULL, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) != 0);
    CUTE_ASSERT(plbuf_edit_add(&plbuf, NULL, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) != 0);
    CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, NULL, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) != 0);
    CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, 0,
                                   test->passwd, strlen(test->passwd)) != 0);
    CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   NULL, strlen(test->passwd)) != 0);
    CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, 0) != 0);

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) == 0);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_find(NULL, plbuf_size, "cern", 3) == 0);
    CUTE_ASSERT(plbuf_edit_find(plbuf, 0, "cern", 3) == 0);
    CUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, NULL, 3) == 0);
    CUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, "cern", 0) == 0);

    CUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, "404", 3) == 0);

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_find(plbuf, plbuf_size, test->alias, strlen(test->alias)) == 1);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "404", 3) != 0);

    test = &test_vector[0];

    CUTE_ASSERT(plbuf_edit_del(NULL, &plbuf_size, test->alias, strlen(test->alias)) != 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, 0, test->alias, strlen(test->alias)) != 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, NULL, strlen(test->alias)) != 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, 0) != 0);

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, strlen(test->alias)) == 0);
        test++;
    }

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) == 0);
        test++;
    }

    test = test_end - 1;

    while (test != (&test_vector[0] - 1)) {
        CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, test->alias, strlen(test->alias)) == 0);
        test--;
    }

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) == 0);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "cern", strlen("cern")) == 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "home", strlen("home")) == 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "nasa", strlen("nasa")) == 0);

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) == 0);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "cern", strlen("cern")) == 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "nasa", strlen("nasa")) == 0);
    CUTE_ASSERT(plbuf_edit_del(&plbuf, &plbuf_size, "home", strlen("home")) == 0);

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(plbuf_edit_add(&plbuf, &plbuf_size, test->alias, strlen(test->alias),
                                   test->passwd, strlen(test->passwd)) == 0);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_shuffle(&plbuf, &plbuf_size) == 0);
    CUTE_ASSERT(plbuf_edit_detach(&plbuf, &plbuf_size) == 0);

    test = &test_vector[0];

    CUTE_ASSERT(plbuf_edit_passwd(NULL, plbuf_size, test->alias, strlen(test->alias), &passwd_size) == NULL);
    CUTE_ASSERT(plbuf_edit_passwd(plbuf, 0, test->alias, strlen(test->alias), &passwd_size) == NULL);
    CUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, NULL, strlen(test->alias), &passwd_size) == NULL);
    CUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, test->alias, 0, &passwd_size) == NULL);
    CUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, test->alias, strlen(test->alias), NULL) == NULL);

    while (test != test_end) {
        passwd = plbuf_edit_passwd(plbuf, plbuf_size, test->alias, strlen(test->alias), &passwd_size);
        CUTE_ASSERT(passwd != NULL);
        CUTE_ASSERT(passwd_size == strlen(test->passwd));
        CUTE_ASSERT(memcmp(passwd, test->passwd, passwd_size) == 0);
        kryptos_freeseg(passwd, passwd_size);
        test++;
    }

    CUTE_ASSERT(plbuf_edit_passwd(plbuf, plbuf_size, "Greenfuzz", strlen("Greenfuzz"), &passwd_size) == NULL);

    kryptos_freeseg(plbuf, plbuf_size);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(crypto_tests)
    zacarias_profiles_ctx *profiles;
    kryptos_u8_t *pwdb = NULL;
    kryptos_u8_t *user = NULL;

    zacarias_profiles_ctx_init(profiles);

    CUTE_ASSERT(zacarias_encrypt_pwdb(NULL, "boo", 3) != 0);
    CUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, NULL, 3) != 0);
    CUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, "boo", 0) != 0);

    CUTE_ASSERT(zacarias_decrypt_pwdb(NULL, "boo", 3) != 0);
    CUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, NULL, 3) != 0);
    CUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, "boo", 0) != 0);

    user = (kryptos_u8_t *) kryptos_newseg(4);
    CUTE_ASSERT(user != NULL);
    pwdb = (kryptos_u8_t *) kryptos_newseg(strlen("passwd\tZm9vYmFy\n"));
    CUTE_ASSERT(pwdb != NULL);
    memcpy(pwdb, "passwd\tZm9vYmFy\n", strlen("passwd\tZm9vYmFy\n"));

    CUTE_ASSERT(zacarias_profiles_ctx_add(&profiles, user, 4, pwdb, strlen(pwdb)) == 0);
    profiles->head->plbuf_size = strlen(pwdb);
    profiles->head->plbuf = (kryptos_u8_t *) kryptos_newseg(profiles->head->plbuf_size + 1);
    CUTE_ASSERT(profiles->head->plbuf != NULL);
    memcpy(profiles->head->plbuf, pwdb, profiles->head->plbuf_size);

    CUTE_ASSERT(zacarias_encrypt_pwdb(&profiles->head, "boo", 3) == 0);
    CUTE_ASSERT(profiles->head->plbuf == NULL);
    CUTE_ASSERT(profiles->head->plbuf_size == 0);
    CUTE_ASSERT(profiles->head->pwdb != NULL);
    CUTE_ASSERT(profiles->head->pwdb_size != 0);
    CUTE_ASSERT(memcmp(profiles->head->pwdb, "passwd\tZm9vYmFy\n", strlen("passwd\tZm9vYmFy\n")) != 0);

    CUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, "Boo", 3) != 0);
    CUTE_ASSERT(profiles->head->plbuf == NULL);
    CUTE_ASSERT(profiles->head->plbuf_size == 0);
    CUTE_ASSERT(profiles->head->pwdb != NULL);
    CUTE_ASSERT(profiles->head->pwdb_size != 0);

    CUTE_ASSERT(zacarias_decrypt_pwdb(&profiles->head, "boo", 3) == 0);
    CUTE_ASSERT(profiles->head->pwdb != NULL);
    CUTE_ASSERT(profiles->head->pwdb_size != 0);
    CUTE_ASSERT(profiles->head->plbuf != NULL);
    CUTE_ASSERT(profiles->head->plbuf_size != 0);
    CUTE_ASSERT(memcmp(profiles->head->plbuf, "passwd\tZm9vYmFy\n", strlen("passwd\tZm9vYmFy\n")) == 0);

    zacarias_profiles_ctx_deinit(profiles);
CUTE_TEST_CASE_END
