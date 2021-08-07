/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <kbd/ptbr.h>
#include <string.h>
#include <stdio.h>

kryptos_u8_t *pt_br_latin1_demuxer(const kryptos_u8_t *input, const size_t input_size, size_t *output_size) {
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpragmas"
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Winvalid-source-encoding"
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpointer-sign"
#endif
    // INFO(Rafael): Sequences in form: "\000<shifted symbol>..." indicates that the shifted symbol will be typed without holding shift key.
    //               In other words, it will anulate the shift key press event. It will only force the key code lookup gets the key code by
    //               using the upper lookup table.
    static struct {
        kryptos_u8_t symbol;
        kryptos_u8_t *key_sequence;
        size_t key_sequence_size;
    } demux_passes[] = {
        { 'á', "\000`a", 3  }, { 'Á', "\000`A", 3  }, { 'à', "`a", 2  }, { 'À', "`A", 2  }, { 'ä', "\"a", 2 }, { 'Ä', "\"A", 2 },
        { 'ã', "~a", 2  }, { 'Ã', "~A", 2  }, { 'â', "^a", 2  }, { 'Â', "^A", 2  },

        { 'é', "\000`e", 3  }, { 'É', "\000`E", 3  }, { 'è', "`e", 2  }, { 'È', "`E", 2  }, { 'ë', "\"e", 2 }, { 'Ë', "\"E", 2 },
        { 'ê', "^e", 2  }, { 'Ê', "^E", 2  },

        { 'í', "\000`i", 3  }, { 'Í', "\000`I", 3  }, { 'ì', "`i", 2  }, { 'Ì', "`I", 2  }, { 'ï', "\"i", 2 }, { 'Ï', "\"I", 2 },
        { 'î', "^i", 2  }, { 'Î', "^I", 2  },

        { 'ó', "\000`o", 3  }, { 'Ó', "\000`O", 3  }, { 'ò', "`o", 2  }, { 'Ò', "`O", 2  }, { 'ö', "\"o", 2 }, { 'Ö', "\"O", 2 },
        { 'õ', "~o", 2  }, { 'Õ', "~O", 2  }, { 'ô', "^o", 2  }, { 'Ô', "^O", 2  },

        { 'ú', "\000`u", 3  }, { 'Ú', "\000`U", 3  }, { 'ù', "`u", 2  }, { 'Ù', "`U", 2  }, { 'ü', "\"u", 2 }, { 'Ü', "\"U", 2 },
        { 'û', "^u", 2  }, { 'Û', "^U", 2  },

        { 'ç', "\000`c", 3  }, { 'Ç', "\000`C", 3  },

        { 'ý', "\000`y", 3  }, { 'Ý', "\000`Y", 3  }, { 'ÿ', "\"y",  2 },
        { '"', "\" ",    2  },

        { 0, NULL, 0   } // INFO(Rafael): This is the sentinel. Do not remove it.
    }, *dm, *dm_end;
    static size_t demux_passes_nr = sizeof(demux_passes) / sizeof(demux_passes[0]);
    const kryptos_u8_t *ip, *ip_end;
    kryptos_u8_t sentinel_sequence[3];
    kryptos_u8_t *output = NULL, *op, *op_end, temp_size;

    if (input == NULL || input_size == 0 || output_size == NULL) {
        return NULL;
    }

    temp_size = input_size << 3;
    output = (kryptos_u8_t *) kryptos_newseg(temp_size);

    if (output == NULL) {
        temp_size = 0;
        return NULL;
    }

    memset(output, 0, temp_size);

    op = output;
    op_end = op + temp_size;

    ip = input;
    ip_end = input + input_size;

    dm_end = &demux_passes[0] + demux_passes_nr;

    while (ip != ip_end && op < op_end) {
        snprintf(sentinel_sequence, sizeof(sentinel_sequence) - 1, "%c", *ip);
        (dm_end - 1)->symbol = *ip;
        (dm_end - 1)->key_sequence = &sentinel_sequence[0];
        (dm_end - 1)->key_sequence_size = strlen(sentinel_sequence);

        dm = &demux_passes[0];
        while (dm != dm_end && *ip != dm->symbol) {
            dm++;
        }

        temp_size = dm->key_sequence_size;

        if ((op + temp_size) < op_end) {
            memcpy(op, dm->key_sequence, temp_size);
            op += temp_size;
        }

        ip++;
    }

    temp_size = 0;
    memset(sentinel_sequence, 0, sizeof(sentinel_sequence));

    if (output != NULL) {
        *output_size = op - output;
        op = op_end = NULL;
    }

    return output;
#if defined(__GNUC__) || defined(__clang__)
# pragma pop
# pragma pop
# pragma pop
#endif
}

kryptos_u8_t pt_br_key_mapper(const kryptos_u8_t k, int *hold_sh) {
    // WARN(Rafael): In order to avoid tricky alignment issues, let's use a flat and straightforward switch.
    switch (k) {
        case '\'':
            *hold_sh = 0;
            return '\'';
        case '1':
            *hold_sh = 0;
            return '1';
        case '2':
            *hold_sh = 0;
            return '2';
        case '3':
            *hold_sh = 0;
            return '3';
        case '4':
            *hold_sh = 0;
            return '4';
        case '5':
            *hold_sh = 0;
            return '5';
        case '6':
            *hold_sh = 0;
            return '6';
        case '7':
            *hold_sh = 0;
            return '7';
        case '8':
            *hold_sh = 0;
            return '8';
        case '9':
            *hold_sh = 0;
            return '9';
        case '0':
            *hold_sh = 0;
            return '0';
        case '-':
            *hold_sh = 0;
            return '-';
        case '=':
            *hold_sh = 0;
            return '=';
        case 'q':
            *hold_sh = 0;
            return 'q';
        case 'w':
            *hold_sh = 0;
            return 'w';
        case 'e':
            *hold_sh = 0;
            return 'e';
        case 'r':
            *hold_sh = 0;
            return 'r';
        case 't':
            *hold_sh = 0;
            return 't';
        case 'y':
            *hold_sh = 0;
            return 'y';
        case 'u':
            *hold_sh = 0;
            return 'u';
        case 'i':
            *hold_sh = 0;
            return 'i';
        case 'o':
            *hold_sh = 0;
            return 'o';
        case 'p':
            *hold_sh = 0;
            return 'p';
        case '[':
            *hold_sh = 0;
            return '[';
        case 'a':
            *hold_sh = 0;
            return 'a';
        case 's':
            *hold_sh = 0;
            return 's';
        case 'd':
            *hold_sh = 0;
            return 'd';
        case 'f':
            *hold_sh = 0;
            return 'f';
        case 'g':
            *hold_sh = 0;
            return 'g';
        case 'h':
            *hold_sh = 0;
            return 'h';
        case 'j':
            *hold_sh = 0;
            return 'j';
        case 'k':
            *hold_sh = 0;
            return 'k';
        case 'l':
            *hold_sh = 0;
            return 'l';
        case '~':
            *hold_sh = 0;
            return '~';
        case ']':
            *hold_sh = 0;
            return ']';
        case '\\':
            *hold_sh = 0;
            return '\\';
        case 'z':
            *hold_sh = 0;
            return 'z';
        case 'x':
            *hold_sh = 0;
            return 'x';
        case 'c':
            *hold_sh = 0;
            return 'c';
        case 'v':
            *hold_sh = 0;
            return 'v';
        case 'b':
            *hold_sh = 0;
            return 'b';
        case 'n':
            *hold_sh = 0;
            return 'n';
        case 'm':
            *hold_sh = 0;
            return 'm';
        case ',':
            *hold_sh = 0;
            return ',';
        case '.':
            *hold_sh = 0;
            return '.';
        case ';':
            *hold_sh = 0;
            return ';';
        case '/':
            *hold_sh = 0;
            return '/';
        case '`':
            *hold_sh = 0;
            return '`';
        case ' ':
            *hold_sh = 0;
            return ' ';

        //case '"':
        //    *hold_sh = 1;
        //    return '\'';
        case '!':
            *hold_sh = 1;
            return '1';
        case '@':
            *hold_sh = 1;
            return '2';
        case '#':
            *hold_sh = 1;
            return '3';
        case '$':
            *hold_sh = 1;
            return '4';
        case '%':
            *hold_sh = 1;
            return '5';
        case '"':
            *hold_sh = 1;
            return '6';
        case '&':
            *hold_sh = 1;
            return '7';
        case '*':
            *hold_sh = 1;
            return '8';
        case '(':
            *hold_sh = 1;
            return '9';
        case ')':
            *hold_sh = 1;
            return '0';
        case '_':
            *hold_sh = 1;
            return '-';
        case '+':
            *hold_sh = 1;
            return '=';
        case 'Q':
            *hold_sh = 1;
            return 'q';
        case 'W':
            *hold_sh = 1;
            return 'w';
        case 'E':
            *hold_sh = 1;
            return 'e';
        case 'R':
            *hold_sh = 1;
            return 'r';
        case 'T':
            *hold_sh = 1;
            return 't';
        case 'Y':
            *hold_sh = 1;
            return 'y';
        case 'U':
            *hold_sh = 1;
            return 'u';
        case 'I':
            *hold_sh = 1;
            return 'i';
        case 'O':
            *hold_sh = 1;
            return 'o';
        case 'P':
            *hold_sh = 1;
            return 'p';
        case '{':
            *hold_sh = 1;
            return '[';
        case 'A':
            *hold_sh = 1;
            return 'a';
        case 'S':
            *hold_sh = 1;
            return 's';
        case 'D':
            *hold_sh = 1;
            return 'd';
        case 'F':
            *hold_sh = 1;
            return 'f';
        case 'G':
            *hold_sh = 1;
            return 'g';
        case 'H':
            *hold_sh = 1;
            return 'h';
        case 'J':
            *hold_sh = 1;
            return 'j';
        case 'K':
            *hold_sh = 1;
            return 'k';
        case 'L':
            *hold_sh = 1;
            return 'l';
        case '^':
            *hold_sh = 1;
            return '~';
        case '}':
            *hold_sh = 1;
            return ']';
        case '|':
            *hold_sh = 1;
            return '\\';
        case 'Z':
            *hold_sh = 1;
            return 'z';
        case 'X':
            *hold_sh = 1;
            return 'x';
        case 'C':
            *hold_sh = 1;
            return 'c';
        case 'V':
            *hold_sh = 1;
            return 'v';
        case 'B':
            *hold_sh = 1;
            return 'b';
        case 'N':
            *hold_sh = 1;
            return 'n';
        case 'M':
            *hold_sh = 1;
            return 'm';
        case '<':
            *hold_sh = 1;
            return ',';
        case '>':
            *hold_sh = 1;
            return '.';
        case ':':
            *hold_sh = 1;
            return ';';
        case '?':
            *hold_sh = 1;
            return '/';

        default:
            return 0;
    }
}
