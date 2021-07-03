/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef KBD_KBD_PTBR_H
#define KBD_KBD_PTBR_H 1

#include <kryptos.h>
#include <stdlib.h>

kryptos_u8_t *pt_br_latin1_demuxer(const kryptos_u8_t *input, const size_t input_size, size_t *output_size);

kryptos_u8_t pt_br_key_mapper(const kryptos_u8_t k, int *hold_sh);

#endif
