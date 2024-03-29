/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#ifndef ZACARIAS_DEV_ACTIONS_H
#define ZACARIAS_DEV_ACTIONS_H 1

#include <defs/io.h>

int zc_dev_act_attach_profile(struct zc_devio_ctx **devio);

int zc_dev_act_detach_profile(struct zc_devio_ctx **devio);

int zc_dev_act_add_password(struct zc_devio_ctx **devio);

int zc_dev_act_del_password(struct zc_devio_ctx **devio);

int zc_dev_act_get_password(struct zc_devio_ctx **devio);

int zc_dev_act_aliases(struct zc_devio_ctx **devio);

//int zc_dev_act_is_sessioned_profile(struct zc_devio_ctx **devio);

//int zc_dev_act_setkey(struct zc_devio_ctx **devio);

#endif
