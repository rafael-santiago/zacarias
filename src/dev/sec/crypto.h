#ifndef ZACARIAS_SEC_CRYPTO_H
#define ZACARIAS_SEC_CRYPTO_H 1

#include <ctx/ctx.h>
#ifndef KRYPTOS_KERNEL_MODE
# include <kryptos.h>
#else
# include <kryptos/kryptos.h>
#endif

int zacarias_decrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size);

int zacarias_encrypt_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size);

int zacarias_setkey_pwdb(zacarias_profile_ctx **profile, const kryptos_u8_t *passwd, const size_t passwd_size,
                         const kryptos_u8_t *new_passwd, const size_t new_passwd_size);

unsigned int unbiased_rand_mod(const unsigned n);

kryptos_u8_t *zacarias_gen_userkey(size_t *size);

#endif
