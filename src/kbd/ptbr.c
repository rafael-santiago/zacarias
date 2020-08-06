#include <kbd/ptbr.h>
#include <string.h>
#include <stdio.h>

kryptos_u8_t *pt_br_latin1_demuxer(const kryptos_u8_t *input, const size_t input_size, size_t *output_size) {
    // INFO(Rafael): Sequences in form: "\000<shifted symbol>..." indicates that the shifted symbol will be typed without holding shift key.
    //               In other words, it will anulate the shift key press event. It will only force the key code lookup gets the key code by
    //               using the upper lookup table.
    static struct {
        kryptos_u8_t symbol;
        kryptos_u8_t *key_sequence;
        size_t key_sequence_size;
    } demux_passes[] = {
        { '�', "\000`a", 3  }, { '�', "\000`A", 3  }, { '�', "`a", 2  }, { '�', "`A", 2  }, { '�', "\"a", 2 }, { '�', "\"A", 2 },
        { '�', "~a", 2  }, { '�', "~A", 2  }, { '�', "^a", 2  }, { '�', "^A", 2  },

        { '�', "\000`e", 3  }, { '�', "\000`E", 3  }, { '�', "`e", 2  }, { '�', "`E", 2  }, { '�', "\"e", 2 }, { '�', "\"E", 2 },
        { '�', "^e", 2  }, { '�', "^E", 2  },

        { '�', "\000`i", 3  }, { '�', "\000`I", 3  }, { '�', "`i", 2  }, { '�', "`I", 2  }, { '�', "\"i", 2 }, { '�', "\"I", 2 },
        { '�', "^i", 2  }, { '�', "^I", 2  },

        { '�', "\000`o", 3  }, { '�', "\000`O", 3  }, { '�', "`o", 2  }, { '�', "`O", 2  }, { '�', "\"o", 2 }, { '�', "\"O", 2 },
        { '�', "~o", 2  }, { '�', "~O", 2  }, { '�', "^o", 2  }, { '�', "^O", 2  },

        { '�', "\000`u", 3  }, { '�', "\000`U", 3  }, { '�', "`u", 2  }, { '�', "`U", 2  }, { '�', "\"u", 2 }, { '�', "\"U", 2 },
        { '�', "^u", 2  }, { '�', "^U", 2  },

        { '�', "\000`c", 3  }, { '�', "\000`C", 3  },

        { '�', "\000`y", 3  }, { '�', "\000`Y", 3  }, { '�', "\"y",  2 },

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
}