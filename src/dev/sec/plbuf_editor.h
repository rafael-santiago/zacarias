/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_SEC_PLBUF_EDITOR_H
#define ZACARIAS_SEC_PLBUF_EDITOR_H 1

#include <kryptos.h>

int plbuf_edit_add(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size,
                   const kryptos_u8_t *passwd, const size_t passwd_size);

int plbuf_edit_del(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size);

int plbuf_edit_find(const kryptos_u8_t *plbuf, const size_t plbuf_size,
                    const kryptos_u8_t *alias, const size_t alias_size);

int plbuf_edit_shuffle_stub(kryptos_u8_t **plbuf, size_t *plbuf_size);

int plbuf_edit_shuffle(kryptos_u8_t **plbuf, size_t *plbuf_size);

int plbuf_edit_detach(kryptos_u8_t **plbuf, size_t *plbuf_size);

kryptos_u8_t *plbuf_edit_passwd(const kryptos_u8_t *plbuf, const size_t plbuf_size,
                                const kryptos_u8_t *alias, const size_t alias_size,
                                size_t *passwd_size);

#endif
