#include <cutest.h>
#include <kbd/kbd.h>
#include <kryptos.h>
#include <string.h>
#include <stdio.h>

CUTE_DECLARE_TEST_CASE(kbd_tests);
CUTE_DECLARE_TEST_CASE(zacarias_getuserkey_tests);
CUTE_DECLARE_TEST_CASE(zacarias_set_kbd_layout_tests);
CUTE_DECLARE_TEST_CASE(zacarias_sendkeys_tests);
CUTE_DECLARE_TEST_CASE(pt_br_latin1_demuxer_tests);

CUTE_MAIN(kbd_tests)

CUTE_TEST_CASE(kbd_tests)
    CUTE_RUN_TEST(zacarias_getuserkey_tests);
    CUTE_RUN_TEST(zacarias_set_kbd_layout_tests);
    CUTE_RUN_TEST(pt_br_latin1_demuxer_tests);
    CUTE_RUN_TEST(zacarias_sendkeys_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zacarias_set_kbd_layout_tests)
    struct {
        const char *name;
        int expected;
    } test_vector[] = {
        { "foo",   0  },
        { "bar",   0  },
        { NULL,    0  },
        { "pt-br", 1  }
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
        CUTE_ASSERT(zacarias_set_kbd_layout(test->name) == test->expected);
        test++;
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zacarias_getuserkey_tests)
    size_t key_size;
    kryptos_u8_t *key;
    setbuf(stdin, NULL); // INFO(Rafael): It will avoid memory leak warnings made by libc.
    CUTE_ASSERT(zacarias_getuserkey(NULL) == NULL);
    key = zacarias_getuserkey(&key_size);
    CUTE_ASSERT(key != NULL);
    CUTE_ASSERT(key_size == 53);
    CUTE_ASSERT(memcmp(key, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.", key_size) == 0);
    kryptos_freeseg(key, key_size);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(zacarias_sendkeys_tests)
    struct {
        kryptos_u8_t *buffer;
        const char *layout;
        int expected;
    } test_vector[] = {
        { NULL,           "pt-br", 1 },
        { "foo",          "pt-br", 0 },
        { "bar",          "pt-br", 0 },
        { "foobar",       "pt-br", 0 },
        { "zacarias",     "pt-br", 0 },
        { "zacarias1234", "pt-br", 0 },
        { "FOO",          "pt-br", 0 },
        { "BAR",          "pt-br", 0 },
        { "ZACARIAS",     "pt-br", 0 },
        { "ZACARIAS1234", "pt-br", 0 },
        { "m�� bode said","pt-br", 0 },
        { "m�� bode said","pt-br", 0 }
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
        CUTE_ASSERT(zacarias_set_kbd_layout(test->layout) == 1);
        CUTE_ASSERT(zacarias_sendkeys(test->buffer, (test->buffer != NULL) ? strlen(test->buffer) : 0, 1) == test->expected);
        test++;
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(pt_br_latin1_demuxer_tests)
    struct {
        kryptos_u8_t *input;
        size_t expected_output_size;
        kryptos_u8_t *expected_output;
    } test_vector[] = {
        { "zacarias", 8, "zacarias" },
        { "�", 2, "`a"  },
        { "�", 2, "`A"  },
        { "�", 2, "`a"  },
        { "�", 2, "`A"  },
        { "�", 2, "\"a" },
        { "�", 2, "\"A" },
        { "�", 2, "~a"  },
        { "�", 2, "~A"  },
        { "�", 2, "`e"  },
        { "�", 2, "`E"  },
        { "�", 2, "`e"  },
        { "�", 2, "`E"  },
        { "�", 2, "\"e" },
        { "�", 2, "\"E" },
        { "�", 2, "`i"  },
        { "�", 2, "`I"  },
        { "�", 2, "`i"  },
        { "�", 2, "`I"  },
        { "�", 2, "\"i" },
        { "�", 2, "\"I" },
        { "�", 2, "`o"  },
        { "�", 2, "`O"  },
        { "�", 2, "`o"  },
        { "�", 2, "`O"  },
        { "�", 2, "\"o" },
        { "�", 2, "\"O" },
        { "�", 2, "~o"  },
        { "�", 2, "~O"  },
        { "�", 2, "`u"  },
        { "�", 2, "`U"  },
        { "�", 2, "`u"  },
        { "�", 2, "`U"  },
        { "�", 2, "\"u" },
        { "�", 2, "\"U" },
        { "�", 2, "`c"  },
        { "�", 2, "`C"  },
        { "�", 2, "'y"  },
        { "�", 2, "'Y"  },
        { "�", 2, "\"y" },
        { "�", 2, "^a"  },
        { "�", 2, "^A"  },
        { "�", 2, "^e"  },
        { "�", 2, "^E"  },
        { "�", 2, "^i"  },
        { "�", 2, "^I"  },
        { "�", 2, "^o"  },
        { "�", 2, "^O"  },
        { "�", 2, "^u"  },
        { "�", 2, "^U"  },
        { "P�ssego", 8, "P^essego" },
        { "Eu digo ei, eu digo ei, c� parece que n�o sei, eu digo ei...", 62,
          "Eu digo ei, eu digo ei, c^e parece que n~ao sei, eu digo ei..." },
        { "�����", 10, "`a`e`i`o`u" }
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);
    kryptos_u8_t *output;
    size_t output_size;

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
        output = pt_br_latin1_demuxer(test->input, strlen(test->input), &output_size);
        CUTE_ASSERT(output_size == test->expected_output_size);
        CUTE_ASSERT(output != NULL);
        CUTE_ASSERT(memcmp(output, test->expected_output, output_size) == 0);
        kryptos_freeseg(output, output_size);
        test++;
    }
CUTE_TEST_CASE_END
