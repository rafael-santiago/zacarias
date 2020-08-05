/*
 *                          Copyright (C) 2020 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_KBD_KBD_H
#define ZACARIAS_KBD_KBD_H 1

#include <kryptos.h>
#include <stdlib.h>
#include <kbd/kmap.h>

kryptos_u8_t *zacarias_getuserkey(size_t *key_size);

int zacarias_set_kbd_layout(const char *name);

int zacarias_sendkeys(const kryptos_u8_t *buffer, const size_t buffer_size, const unsigned int timeout_in_secs);

#endif
