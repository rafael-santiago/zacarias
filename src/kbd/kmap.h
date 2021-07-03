/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_KMAP_H
#define ZACARIAS_KMAP_H 1

#include <kryptos.h>
#include <kbd/ptbr.h>

typedef kryptos_u8_t *(*key_demux_func_t)(const kryptos_u8_t *input, const size_t input_size, size_t *output_size);

typedef kryptos_u8_t (*key_mapper_func_t)(const kryptos_u8_t k, int *hold_sh);

typedef struct {
    char *name;
    key_demux_func_t key_demuxer;
    key_mapper_func_t key_mapper;
}zacarias_kbd_layout;

typedef struct {
    zacarias_kbd_layout layout;
}zacarias_kmap_t;

static zacarias_kmap_t gZacariasKmap[] = {
    { "pt-br", pt_br_latin1_demuxer, pt_br_key_mapper }
};

static size_t gZacariasKmapNr = sizeof(gZacariasKmap) / sizeof(gZacariasKmap[0]);

extern zacarias_kbd_layout *gZacariasCurrKbdLayout;

#endif
