/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cutest.h>
#include <kryptos.h>
#include <ctx/ctx.h>
#include <dev/defs/io.h>

CUTE_DECLARE_TEST_CASE(ctx_tests);
CUTE_DECLARE_TEST_CASE(ctx_general_tests);

CUTE_MAIN(ctx_tests);

CUTE_TEST_CASE(ctx_tests)
    CUTE_RUN_TEST(ctx_general_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(ctx_general_tests)
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpointer-sign"
#endif
    zacarias_profiles_ctx *profiles;
    struct {
        char *user;
        char *pwdb_path;
        kryptos_u8_t *pwdb;
        int expected;
    } test_vector[] = {
        { "foo", "foo.db", "<foo's data>", 0 },
        { "rs", "rs.db", "<rs' data>", 0 },
        { "godofredo", "godofredo.db", "<godofredo's data>", 0 },
        { "lobato", "lobato.db", "<lobato's data>", 0 },
        { "foo", "foo.db", "<foo's data>", 1 },
        { "rs", "rs.db",  "<rs' data>", 1 },
        { "godofredo", "godofredo.db", "<godofredo's data>", 1 },
        { "lobato", "lobato.db", "<lobato's data>", 1 }
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);
    char user[ZC_STR_NR];
    size_t user_size;
    char pwdb_path[ZC_STR_NR];
    size_t pwdb_path_size;
    kryptos_u8_t pwdb[ZC_STR_NR];
    size_t pwdb_size;
    zacarias_profile_ctx *profile;

    zacarias_profiles_ctx_init(profiles);
    CUTE_ASSERT(profiles != NULL);

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
        user_size = strlen(test->user);
        memcpy(user, test->user, user_size);
        pwdb_path_size = strlen(test->pwdb_path);
        memcpy(pwdb_path, test->pwdb_path, pwdb_path_size);
        pwdb_size = strlen(test->pwdb);
        memcpy(pwdb, test->pwdb, pwdb_size);
        CUTE_ASSERT(zacarias_profiles_ctx_add(&profiles, user, user_size, pwdb_path, pwdb_path_size,
                                              pwdb, pwdb_size) == test->expected);
        if (test->expected == 0) {
            CUTE_ASSERT(profiles->tail != NULL && profiles->head != NULL);
            CUTE_ASSERT(profiles->tail->user_size == user_size);
            CUTE_ASSERT(memcmp(profiles->tail->user, user, user_size) == 0);
            CUTE_ASSERT(profiles->tail->pwdb_path_size == pwdb_path_size);
            CUTE_ASSERT(memcmp(profiles->tail->pwdb_path, pwdb_path, pwdb_path_size) == 0);
            CUTE_ASSERT(profiles->tail->pwdb_size == pwdb_size);
            CUTE_ASSERT(memcmp(profiles->tail->pwdb, pwdb, pwdb_size) == 0);
            CUTE_ASSERT(profiles->tail->sessioned == 0);
        }
        test++;
    }

    CUTE_ASSERT(zacarias_profiles_ctx_get(profiles, "404", 3) == NULL);

    test = &test_vector[0];

    while (test != test_end) {
        user_size = strlen(test->user);
        profile = zacarias_profiles_ctx_get(profiles, test->user, user_size);
        CUTE_ASSERT(profile != NULL);
        pwdb_path_size = strlen(test->pwdb_path);
        pwdb_size = strlen(test->pwdb);
        CUTE_ASSERT(profile->user_size == user_size);
        CUTE_ASSERT(memcmp(profile->user, test->user, user_size) == 0);
        CUTE_ASSERT(profile->pwdb_path_size == pwdb_path_size);
        CUTE_ASSERT(memcmp(profile->pwdb_path, test->pwdb_path, pwdb_path_size) == 0);
        CUTE_ASSERT(profile->pwdb_size == pwdb_size);
        CUTE_ASSERT(memcmp(profile->pwdb, test->pwdb, pwdb_size) == 0);
        test++;
    }

    CUTE_ASSERT(zacarias_profiles_ctx_del(&profiles, "404", 3) == 1);

    test = &test_vector[0];
    while (test != test_end) {
        CUTE_ASSERT(zacarias_profiles_ctx_del(&profiles, test->user, strlen(test->user)) == test->expected);
        test++;
    }

    zacarias_profiles_ctx_deinit(profiles);
#if defined(__GNUC__) || defined(__clang__)
# pragma pop
#endif
CUTE_TEST_CASE_END
