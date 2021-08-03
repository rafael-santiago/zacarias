/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cutest.h>
#include <libc/memset.h>
#include <libc/memcpy.h>
#include <libc/memcmp.h>
#include <stdlib.h>

CUTE_DECLARE_TEST_CASE(zc_memset_tests);
CUTE_DECLARE_TEST_CASE(zc_memcpy_tests);
CUTE_DECLARE_TEST_CASE(zc_memcmp_tests);

CUTE_DECLARE_TEST_CASE(zc_tests);

CUTE_MAIN(zc_tests);

CUTE_TEST_CASE(zc_tests)
    CUTE_RUN_TEST(zc_memset_tests);
    CUTE_RUN_TEST(zc_memcmp_tests);
    CUTE_RUN_TEST(zc_memcpy_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zc_memset_tests)
    char abc[4] = { 1, 2, 3, 4 };
    size_t a;
    zc_memset(abc, 0, sizeof(abc));
    for (a = 0; a < sizeof(abc); a++) {
        CUTE_ASSERT(abc[a] == 0);
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zc_memcmp_tests)
    struct test_ctx {
        char *a;
        char *b;
        size_t size;
        int equal;
    } test_vector[] = {
        { "Raja Haje", "raja Haje", 9, 0 },
        { "Raja Haje", "RAja Haje", 9, 0 },
        { "Raja Haje", "RaJa Haje", 9, 0 },
        { "Raja Haje", "RajA Haje", 9, 0 },
        { "Raja Haje", "Raja Haje", 9, 1 },
    }, *test = &test_vector[0], *test_end = test + sizeof(test_vector) / sizeof(test_vector[0]);

    while (test != test_end) {
        if (test->equal) {
            CUTE_ASSERT(zc_memcmp(test->a, test->b, test->size) == 0);
        } else {
            CUTE_ASSERT(zc_memcmp(test->a, test->b, test->size) != 0);
        }
        test++;
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zc_memcpy_tests)
    char src[] = "Deep In The Sand";
    char dest[16] = "";
    zc_memcpy(dest, src, 15);
    CUTE_ASSERT(zc_memcmp(dest, src, 15) == 0);
CUTE_TEST_CASE_END
