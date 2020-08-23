#include <cutest.h>
#include <sec/crypto.h>
#include <ctx/ctx.h>
#include <string.h>

CUTE_DECLARE_TEST_CASE(sec_tests);
CUTE_DECLARE_TEST_CASE(crypto_tests);

CUTE_MAIN(sec_tests);

CUTE_TEST_CASE(sec_tests)
    CUTE_RUN_TEST(crypto_tests);
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
