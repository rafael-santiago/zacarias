#ifndef ZACARIAS_SEC_CRYPTO_H
#define ZACARIAS_SEC_CRYPTO_H 1

#include <ctx/ctx.h>
#include <kryptos.h>

int zacarias_decrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size);

int zacarias_encrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size);

#endif
