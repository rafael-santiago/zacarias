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
        { "méé bode said","pt-br", 0 },
        { "mèè bode said","pt-br", 0 }
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
        { "á", 2, "`a"  },
        { "Á", 2, "`A"  },
        { "à", 2, "`a"  },
        { "À", 2, "`A"  },
        { "ä", 2, "\"a" },
        { "Ä", 2, "\"A" },
        { "ã", 2, "~a"  },
        { "Ã", 2, "~A"  },
        { "é", 2, "`e"  },
        { "É", 2, "`E"  },
        { "è", 2, "`e"  },
        { "È", 2, "`E"  },
        { "ë", 2, "\"e" },
        { "Ë", 2, "\"E" },
        { "í", 2, "`i"  },
        { "Í", 2, "`I"  },
        { "ì", 2, "`i"  },
        { "Ì", 2, "`I"  },
        { "ï", 2, "\"i" },
        { "Ï", 2, "\"I" },
        { "ó", 2, "`o"  },
        { "Ó", 2, "`O"  },
        { "ò", 2, "`o"  },
        { "Ò", 2, "`O"  },
        { "ö", 2, "\"o" },
        { "Ö", 2, "\"O" },
        { "õ", 2, "~o"  },
        { "Õ", 2, "~O"  },
        { "ú", 2, "`u"  },
        { "Ú", 2, "`U"  },
        { "ù", 2, "`u"  },
        { "Ù", 2, "`U"  },
        { "ü", 2, "\"u" },
        { "Ü", 2, "\"U" },
        { "ç", 2, "`c"  },
        { "Ç", 2, "`C"  },
        { "ý", 2, "'y"  },
        { "Ý", 2, "'Y"  },
        { "ÿ", 2, "\"y" },
        { "â", 2, "^a"  },
        { "Â", 2, "^A"  },
        { "ê", 2, "^e"  },
        { "Ê", 2, "^E"  },
        { "î", 2, "^i"  },
        { "Î", 2, "^I"  },
        { "ô", 2, "^o"  },
        { "Ô", 2, "^O"  },
        { "û", 2, "^u"  },
        { "Û", 2, "^U"  },
        { "Pêssego", 8, "P^essego" },
        { "Eu digo ei, eu digo ei, cê parece que não sei, eu digo ei...", 62,
          "Eu digo ei, eu digo ei, c^e parece que n~ao sei, eu digo ei..." },
        { "áéíóú", 10, "`a`e`i`o`u" }
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
