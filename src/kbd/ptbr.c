#include <kbd/ptbr.h>
#include <string.h>
#include <stdio.h>

kryptos_u8_t *pt_br_latin1_demuxer(kryptos_u8_t *input, const size_t input_size, size_t *output_size) {
    static struct {
        kryptos_u8_t symbol;
        kryptos_u8_t *key_sequence;
    } demux_passes[] = {
        { '�', "'a"  }, { '�', "'A"  }, { '�', "`a"  }, { '�', "`A"  }, { '�', "\"a" }, { '�', "\"A" }, { '�', "~a"  }, { '�', "~A"  },
        { '�', "'e"  }, { '�', "'E"  }, { '�', "`e"  }, { '�', "`E"  }, { '�', "\"e" }, { '�', "\"E" },
        { '�', "'i"  }, { '�', "'I"  }, { '�', "`i"  }, { '�', "`I"  }, { '�', "\"i" }, { '�', "\"I" },
        { '�', "'o"  }, { '�', "'O"  }, { '�', "`o"  }, { '�', "`O"  }, { '�', "\"o" }, { '�', "\"O" }, { '�', "~o"  }, { '�', "~O"  },
        { '�', "'u"  }, { '�', "'U"  }, { '�', "`u"  }, { '�', "`U"  }, { '�', "\"u" }, { '�', "\"U" },
        { '�', "'c"  }, { '�', "'C"  },
        { '�', "'y"  }, { '�', "'Y"  }, { '�', "\"y" },
        { 0, NULL    } // INFO(Rafael): This is the sentinel. Do not remove it.
    }, *dm, *dm_end;
    static size_t demux_passes_nr = sizeof(demux_passes) / sizeof(demux_passes[0]);
    kryptos_u8_t *ip, *ip_end;
    kryptos_u8_t sentinel_sequence[3];
    kryptos_u8_t *output = NULL, *op, *op_end, temp_size;

    if (input == NULL || input_size == 0 || output_size == NULL) {
        return NULL;
    }

    temp_size = input_size << 1;
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

        dm = &demux_passes[0];
        while (dm != dm_end && *ip == dm->symbol) {
            dm++;
        }

        temp_size = strlen(dm->key_sequence);

        if ((op + temp_size) < op_end) {
            memcpy(op, dm->key_sequence, temp_size);
            op += temp_size;
        }

        ip++;
    }

    temp_size = 0;
    memset(sentinel_sequence, 0, sizeof(sentinel_sequence));

    return output;
}