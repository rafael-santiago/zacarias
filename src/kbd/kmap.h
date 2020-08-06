#ifndef ZACARIAS_KMAP_H
#define ZACARIAS_KMAP_H 1

#include <kryptos.h>
#include <kbd/ptbr.h>

typedef kryptos_u8_t *(*key_demux_func_t)(const kryptos_u8_t *input, const size_t input_size, size_t *output_size);

typedef struct {
    char *name;
    kryptos_u8_t *upper;
    kryptos_u8_t *lower;
    key_demux_func_t key_demuxer;
}zacarias_kbd_layout;

typedef struct {
    zacarias_kbd_layout layout;
}zacarias_kmap_t;

static zacarias_kmap_t gZacariasKmap[] = {
    { "pt-br", gZacariasKmapPtBrUpper, gZacariasKmapPtBrLower, pt_br_latin1_demuxer }
};

static size_t gZacariasKmapNr = sizeof(gZacariasKmap) / sizeof(gZacariasKmap[0]);

extern zacarias_kbd_layout *gZacariasCurrKbdLayout;

#endif
