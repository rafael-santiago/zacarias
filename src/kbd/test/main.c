/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cutest.h>
#include <kbd/kbd.h>
#include <kryptos.h>
#include <string.h>
#include <stdio.h>
#if defined(__unix__)
# include <X11/Xlib.h>
#endif // defined(__unix__)

CUTE_DECLARE_TEST_CASE(kbd_tests);
CUTE_DECLARE_TEST_CASE(zacarias_getuserkey_tests);
#if defined(__unix__)
CUTE_DECLARE_TEST_CASE(zacarias_set_kbd_layout_tests);
CUTE_DECLARE_TEST_CASE(pt_br_latin1_demuxer_tests);
#endif
CUTE_DECLARE_TEST_CASE(zacarias_sendkeys_tests);

CUTE_MAIN(kbd_tests)

CUTE_TEST_CASE(kbd_tests)
    CUTE_RUN_TEST(zacarias_getuserkey_tests);
#if defined(__unix__)
    CUTE_RUN_TEST(zacarias_set_kbd_layout_tests);
    CUTE_RUN_TEST(pt_br_latin1_demuxer_tests);
#endif // defined(__unix__)
    CUTE_RUN_TEST(zacarias_sendkeys_tests);
CUTE_TEST_CASE_END

#if defined(__unix__)

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

CUTE_TEST_CASE(pt_br_latin1_demuxer_tests)
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpragmas"
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpointer-sign"
# pragma GCC diagnostic ignored "-Winvalid-source-encoding"
#endif
    struct {
        kryptos_u8_t *input;
        size_t expected_output_size;
        kryptos_u8_t *expected_output;
    } test_vector[] = {
        { "zacarias", 8, "zacarias" },
        { "·", 3, "\000`a"  },
        { "¡", 3, "\000`A"  },
        { "‡", 2, "`a"      },
        { "¿", 2, "`A"      },
        { "‰", 2, "\"a"     },
        { "ƒ", 2, "\"A"     },
        { "„", 2, "~a"      },
        { "√", 2, "~A"      },
        { "È", 3, "\000`e"  },
        { "…", 3, "\000`E"  },
        { "Ë", 2, "`e"      },
        { "»", 2, "`E"      },
        { "Î", 2, "\"e"     },
        { "À", 2, "\"E"     },
        { "Ì", 3, "\000`i"  },
        { "Õ", 3, "\000`I"  },
        { "Ï", 2, "`i"      },
        { "Ã", 2, "`I"      },
        { "Ô", 2, "\"i"     },
        { "œ", 2, "\"I"     },
        { "Û", 3, "\000`o"  },
        { "”", 3, "\000`O"  },
        { "Ú", 2, "`o"      },
        { "“", 2, "`O"      },
        { "ˆ", 2, "\"o"     },
        { "÷", 2, "\"O"     },
        { "ı", 2, "~o"      },
        { "’", 2, "~O"      },
        { "˙", 3, "\000`u"  },
        { "⁄", 3, "\000`U"  },
        { "˘", 2, "`u"      },
        { "Ÿ", 2, "`U"      },
        { "¸", 2, "\"u"     },
        { "‹", 2, "\"U"     },
        { "Á", 3, "\000`c"  },
        { "«", 3, "\000`C"  },
        { "˝", 3, "\000`y"  },
        { "›", 3, "\000`Y"  },
        { "ˇ", 2, "\"y"     },
        { "‚", 2, "^a"      },
        { "¬", 2, "^A"      },
        { "Í", 2, "^e"      },
        { " ", 2, "^E"      },
        { "Ó", 2, "^i"      },
        { "Œ", 2, "^I"      },
        { "Ù", 2, "^o"      },
        { "‘", 2, "^O"      },
        { "˚", 2, "^u"      },
        { "€", 2, "^U"      },
        { "PÍssego", 8, "P^essego" },
        { "Eu digo ei, eu digo ei, cÍ parece que n„o sei, eu digo ei...", 62,
          "Eu digo ei, eu digo ei, c^e parece que n~ao sei, eu digo ei..." },
        { "·ÈÌÛ˙", 15, "\000`a\000`e\000`i\000`o\000`u" }
    }, *test, *test_end;
#if defined(__GNUC__) || defined(__clang__)
# pragma pop
# pragma pop
# pragma pop
#endif
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

#endif // defined(__unix__)

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
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpragmas"
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpointer-sign"
# pragma GCC diagnostic ignored "-Winvalid-source-encoding"
#endif
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
        { "mÈÈ bode said","pt-br", 0 },
        { "‚¬Í ÓŒÙ‘˚€",   "pt-br", 0 },
        { "„√ı’",         "pt-br", 0 },
        { "Á«·¡È…ÌÕÛ”˙⁄", "pt-br", 0 },
        { "‡¿Ë»ÏÃÚ“˘Ÿ",   "pt-br", 0 },
        { "‰ƒÎÀÔœˆ÷¸‹",   "pt-br", 0 },
        { "!?:;,%$&@#\"'*<>().", "pt-br", 0 }
    }, *test, *test_end;
    size_t test_vector_nr = sizeof(test_vector) / sizeof(test_vector[0]);
#if defined(__GNUC__) || defined(__clang__)
# pragma pop
# pragma pop
# pragma pop
#endif

#if defined(__FreeBSD__)
    // WARN(Rafael): On FreeBSD (12.1-RELEASE) I have noticed a memory leak inside libthr related to a condition structure (thread/thr_cond.c:103).
    //               It is accessed and initialized by an indirect call from libxcb. This is passing on Linux without any memory leak issue. Thus,
    //                I will trust on my Linux environment and disable it by now on FreeBSD.
    if (CUTE_GET_OPTION("cutest-leak-check") != NULL) {
        g_cute_leak_check = 0;
    }
#endif // defined(__FreeBSD__)

    test = &test_vector[0];
    test_end = test + test_vector_nr;

    while (test != test_end) {
#if defined(__unix__)
        CUTE_ASSERT(zacarias_set_kbd_layout(test->layout) == 1);
#endif // defined(__unix__)
        CUTE_ASSERT(zacarias_sendkeys(test->buffer, (test->buffer != NULL) ? strlen(test->buffer) : 0,
                                      1, NULL, NULL) == test->expected);
        test++;
    }

#if defined(__FreeBSD__)
    if (CUTE_GET_OPTION("cutest-leak-check") != NULL) {
        g_cute_leak_check = 1;
    }
#endif // defined(__FreeBSD__)
CUTE_TEST_CASE_END
