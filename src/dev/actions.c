/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <defs/io.h>
#include <defs/types.h>
#include <defs/zc_dbg.h>
#if defined(__linux__)
# include <linux/slab.h>
# include <asm/uaccess.h>
#endif
#include <kio.h>
#include <ctx/ctx.h>
#include <sec/crypto.h>
#include <sec/plbuf_editor.h>

int zc_dev_act_add_password(struct zc_devio_ctx **devio) {
    int err = EFAULT;
    struct zc_devio_ctx *d = *devio;
    zacarias_profile_ctx *profile;
    unsigned char *passwd = NULL;
    size_t passwd_size = 0;
    unsigned char detached = 0;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }

    if (d->user_size == 0 || d->pwdb_passwd_size == 0 || d->alias_size == 0) {
        d->status = kInvalidParams;
        err = EINVAL;
        ZC_DBG("invalid size(s) [d->user_size=%lu, d->pwdb_passwd_size=%lu, d->alias_size=%lu]\n",
               d->user_size, d->pwdb_passwd_size, d->alias_size);
        goto zc_dev_act_add_password_epilogue;
    }

    if ((profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size)) == NULL) {
        d->status = kProfileNotAttached;
        err = 0;
        ZC_DBG("profile not found.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (profile->sessioned && d->session_passwd_size == 0) {
        d->status = kInvalidParams;
        err = EINVAL;
        ZC_DBG("sessioned profile but session password not provided.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (profile->sessioned) {
        // INFO(Rafael): Dirty trick. This setkey is only for validating the session key. We need to reload it
        //               from disk in order to validate the master key, too.
        if (zacarias_setkey_pwdb(&profile,
                                 d->session_passwd, d->session_passwd_size,
                                 d->pwdb_passwd, d->pwdb_passwd_size) == 0) {
            kryptos_freeseg(profile->pwdb, profile->pwdb_size);
            profile->pwdb_size = 0;
            if (kread(profile->pwdb_path, &profile->pwdb, &profile->pwdb_size) != 0) {
                d->status = kPWDBReadingError;
                err = 0;
                ZC_DBG("kread has failed.\n");
                goto zc_dev_act_add_password_epilogue;
            }
        } else {
            d->status = kAuthenticationFailure;
            err = 0;
            ZC_DBG("zacarias_setkey_pwdb() has failed.\n");
            goto zc_dev_act_add_password_epilogue;
        }
    }

    if (zacarias_decrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        d->status = kAuthenticationFailure;
        err = 0;
        ZC_DBG("zacarias_decrypt_pwdb() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (plbuf_edit_detach(&profile->plbuf, &profile->plbuf_size) != 0) {
        d->status = kGeneralError;
        err = 0;
        ZC_DBG("plbuf_edit_detach() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    detached = 1;

    if (plbuf_edit_find(profile->plbuf, profile->plbuf_size, d->alias, d->alias_size)) {
        err = 0;
        d->status = kAliasAlreadyUsed;
        ZC_DBG("plbuf_edit_find() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (d->passwd_size == 0) {
        passwd = zacarias_gen_userkey(&passwd_size);
        if (passwd == NULL) {
            err = ENOMEM;
            d->status = kGeneralError;
            ZC_DBG("null password.\n");
            goto zc_dev_act_add_password_epilogue;
        }
    } else {
        passwd = &d->passwd[0];
        passwd_size = d->passwd_size;
    }

    if (plbuf_edit_add(&profile->plbuf, &profile->plbuf_size, d->alias, d->alias_size, passwd, passwd_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        ZC_DBG("plbuf_edit_add() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (plbuf_edit_shuffle(&profile->plbuf, &profile->plbuf_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        ZC_DBG("plbuf_edit_shuffle() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    detached = 0;

    if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        ZC_DBG("zacarias_encrypt_pwdb() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (kwrite(profile->pwdb_path, profile->pwdb, profile->pwdb_size) != 0) {
        err = 0;
        d->status = kPWDBWritingError;
        ZC_DBG("kwrite() has failed.\n");
        goto zc_dev_act_add_password_epilogue;
    }

    if (profile->sessioned) {
        if (zacarias_setkey_pwdb(&profile,
                                 d->pwdb_passwd, d->pwdb_passwd_size,
                                 d->session_passwd, d->session_passwd_size) != 0) {
            err = 0;
            d->status = kGeneralError;
            ZC_DBG("zacarias_setkey_pwdb() has failed.\n");
            goto zc_dev_act_add_password_epilogue;
        }
    }

    err = 0;
    d->status = kNoError;

zc_dev_act_add_password_epilogue:

    if (d->status != kNoError && profile != NULL) {
        if (detached) {
            if (plbuf_edit_shuffle(&profile->plbuf, &profile->plbuf_size) == 0) {
                if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) == 0) {
                    if (profile->sessioned) {
                        zacarias_setkey_pwdb(&profile,
                                             d->pwdb_passwd, d->pwdb_passwd_size,
                                             d->session_passwd, d->session_passwd_size);
                    }
                }
            }
        } else {
            if (d->pwdb_passwd_size > 0 && profile->plbuf != NULL) {
                if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) == 0) {
                    if (profile->sessioned && d->session_passwd_size > 0) {
                        zacarias_setkey_pwdb(&profile,
                                             d->pwdb_passwd, d->pwdb_passwd_size,
                                             d->session_passwd, d->session_passwd_size);
                    }
                }
            } else if (profile->sessioned && d->session_passwd_size > 0 && profile->plbuf != NULL) {
                zacarias_encrypt_pwdb(&profile, d->session_passwd, d->session_passwd_size);
            }
        }
    }

    if (passwd != NULL && passwd != &d->passwd[0]) {
        kryptos_freeseg(passwd, passwd_size);
    }

    passwd = NULL;

    passwd_size = 0;
    profile = NULL;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_del_password(struct zc_devio_ctx **devio) {
    struct zc_devio_ctx *d = *devio;
    int err = EFAULT;
    zacarias_profile_ctx *profile = NULL;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }

    if (d->alias == NULL || d->alias_size == 0 || d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0 || d->user == NULL ||
        d->user_size == 0) {
        err = 0;
        d->status = kInvalidParams;
        ZC_DBG("invalid parameters.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size);
    if (profile == NULL) {
        err = 0;
        d->status = kProfileNotAttached;
        ZC_DBG("profile not found.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (profile->sessioned && d->session_passwd_size > 0) {
        if (zacarias_setkey_pwdb(&profile,
                                 d->session_passwd, d->session_passwd_size, d->pwdb_passwd, d->pwdb_passwd_size) == 0) {
            kryptos_freeseg(profile->pwdb, profile->pwdb_size);
            profile->pwdb = NULL;
            profile->pwdb_size = 0;
            if (kread(profile->pwdb_path, &profile->pwdb, &profile->pwdb_size) != 0) {
                err = 0;
                d->status = kPWDBReadingError;
                ZC_DBG("kread has failed.\n");
                goto zc_dev_act_del_password_epilogue;
            }
        } else {
            err = 0;
            d->status = kAuthenticationFailure;
            ZC_DBG("zacarias_setkey_pwdb() has failed.\n");
            goto zc_dev_act_del_password_epilogue;
        }
    } else if (profile->sessioned && d->session_passwd_size == 0) {
        err = 0;
        d->status = kInvalidParams;
        ZC_DBG("sessioned profile with no session password.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (zacarias_decrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        err = 0;
        d->status = kAuthenticationFailure;
        ZC_DBG("zacarias_decrypt_pwdb() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (plbuf_edit_detach(&profile->plbuf, &profile->plbuf_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size);
        ZC_DBG("plbuf_edit_detach() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (plbuf_edit_del(&profile->plbuf, &profile->plbuf_size, d->alias, d->alias_size) != 0) {
        err = 0;
        d->status = kAliasNotFound;
        ZC_DBG("plbuf_edit_del() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (profile->plbuf_size > 0 && plbuf_edit_shuffle(&profile->plbuf, &profile->plbuf_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        kryptos_freeseg(profile->plbuf, profile->plbuf_size);
        profile->plbuf = NULL;
        profile->plbuf_size = 0;
        ZC_DBG("plbuf_edit_shuffle() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    } else if (profile->plbuf_size == 0) {
        if (plbuf_edit_add(&profile->plbuf, &profile->plbuf_size,
                           "\x1BZ", 2, "\x1BZ", 2) != 0) {
            d->status = kPWDBWritingError;
            ZC_DBG("plbuf_edit_add() has failed.\n");
            goto zc_dev_act_del_password_epilogue;
        }
        if (plbuf_edit_shuffle_stub(&profile->plbuf, &profile->plbuf_size) != 0) {
            d->status = kGeneralError;
            ZC_DBG("plbuf_edit_shuffle_stub() has failed.\n");
            goto zc_dev_act_del_password_epilogue;
        }
    }

    if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        ZC_DBG("zacarias_encrypt_pwdb() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (kwrite(profile->pwdb_path, profile->pwdb, profile->pwdb_size) != 0) {
        err = 0;
        d->status = kPWDBWritingError;
        ZC_DBG("kwrite() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    if (profile->sessioned && d->session_passwd_size > 0 && zacarias_setkey_pwdb(&profile,
                                                      d->pwdb_passwd, d->pwdb_passwd_size,
                                                d->session_passwd, d->session_passwd_size) != 0) {
        err = 0;
        d->status = kAuthenticationFailure;
        ZC_DBG("zacarias_setkey_pwdb() has failed.\n");
        goto zc_dev_act_del_password_epilogue;
    }

    d->status = kNoError;
    err = 0;

zc_dev_act_del_password_epilogue:

    if (profile != NULL && profile->plbuf != NULL) {
        kryptos_freeseg(profile->plbuf, profile->plbuf_size);
        profile->plbuf = NULL;
        profile->plbuf_size = 0;
    }

    d = NULL;
    profile = NULL;
    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_get_password(struct zc_devio_ctx **devio) {
    int err = EFAULT;
    struct zc_devio_ctx *d = *devio;
    zacarias_profile_ctx *profile = NULL;
    unsigned char *passwd = NULL;
    size_t passwd_size = 0;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }

    if (d->alias_size == 0 || d->user_size == 0) {
        err = 0;
        d->status = kInvalidParams;
        ZC_DBG("invalid sizes (d->alias_size=%lu, d->user_size=%lu).\n", d->alias_size, d->user_size);
        goto zc_dev_act_get_password_epilogue;
    }

    profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size);
    if (profile == NULL) {
        err = 0;
        d->status = kProfileNotAttached;
        ZC_DBG("profile not found.\n");
        goto zc_dev_act_get_password_epilogue;
    }

    if (zacarias_decrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        err = 0;
        d->status = kAuthenticationFailure;
        ZC_DBG("zacarias_decrypt_pwdb() has failed.\n");
        goto zc_dev_act_get_password_epilogue;
    }

    if (plbuf_edit_detach(&profile->plbuf, &profile->plbuf_size) != 0) {
        err = 0;
        d->status = kGeneralError;
        zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size);
        ZC_DBG("zacarias_edit_detach() has failed.\n");
        goto zc_dev_act_get_password_epilogue;
    }

    passwd = plbuf_edit_passwd(profile->plbuf, profile->plbuf_size, d->alias, d->alias_size, &passwd_size);
    d->status = (passwd != NULL) ? kNoError : kAliasNotFound;

    if (d->status == kNoError) {
        d->passwd_size = (passwd_size > ZC_STR_NR) ? ZC_STR_NR : passwd_size;
        memcpy(d->passwd, passwd, d->passwd_size);
    }

    if (plbuf_edit_shuffle(&profile->plbuf, &profile->plbuf_size) != 0) {
        err = 0;
        kryptos_freeseg(profile->plbuf, profile->plbuf_size);
        profile->plbuf = NULL;
        profile->plbuf_size = 0;
        ZC_DBG("zacarias_edit_shuffle() has failed.\n");
        goto zc_dev_act_get_password_epilogue;
    }

    zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size);
    err = 0;

zc_dev_act_get_password_epilogue:

    if (passwd != NULL) {
        kryptos_freeseg(passwd, passwd_size);
    }

    passwd_size = 0;
    profile = NULL;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}
/*
int zc_dev_act_setkey(struct zc_devio_ctx **devio) {
    int err = EFAULT;
    struct zc_devio_ctx *d = *devio;
    char *user = NULL;
    size_t user_size;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size;
    unsigned char *new_passwd = NULL;
    size_t new_passwd_size;
    zacarias_profile_ctx *profile;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        return EBUSY;
    }

    if (d->user == NULL || d->user_size == 0 || d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0 ||
        d->passwd == NULL || d->passwd_size == 0) {
        err = EINVAL;
        d->status = kInvalidParams;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    user_size = d->user_size;
    user = (char *) kryptos_newseg(user_size);
    if (user == NULL) {
        err = ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    pwdb_passwd_size = d->pwdb_passwd_size;
    pwdb_passwd = (unsigned char *) kryptos_newseg(pwdb_passwd_size);
    if (pwdb_passwd == NULL) {
        err = ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    new_passwd_size = d->passwd_size;
    new_passwd = (unsigned char *) kryptos_newseg(new_passwd_size);
    if (new_passwd == NULL) {
        err = ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    err = 0;

    if ((profile = zacarias_profiles_ctx_get(g_cdev()->profiles, user, user_size)) == NULL) {
        d->status = kProfileNotAttached;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    if (zacarias_setkey_pwdb(&profile, pwdb_passwd, pwdb_passwd_size, new_passwd, new_passwd_size) != 0) {
        d->status = kAuthenticationFailure;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    if (kwrite(profile->pwdb_path, profile->pwdb, profile->pwdb_size) != 0) {
        d->status = kPWDBWritingError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    d->status = kNoError;

zc_dev_act_setkey_profile_epilogue:

    if (user != NULL) {
        memset(user, 0, user_size);
        kryptos_freeseg(user, user_size);
    }

    if (pwdb_passwd != NULL) {
        memset(pwdb_passwd, 0, pwdb_passwd_size);
        kryptos_freeseg(pwdb_passwd, pwdb_passwd_size);
    }

    if (new_passwd != NULL) {
        memset(new_passwd, 0, new_passwd_size);
        kryptos_freeseg(new_passwd, new_passwd_size);
    }

    user_size = pwdb_passwd_size = new_passwd_size = 0;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_is_sessioned_profile(struct zc_devio_ctx **devio) {
    int err = EFAULT;
    char *user = NULL;
    size_t user_size;
    struct zc_devio_ctx *d = *devio;
    zacarias_profile_ctx *profile = NULL;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        return EBUSY;
    }

    if (d->user == NULL || d->user_size == 0) {
        d->status = kInvalidParams;
        err = EINVAL;
        goto zc_dev_act_is_sessioned_profile;
    }

    user_size = d->user_size;
    user = (char *) kryptos_newseg(user_size);
    if (user == NULL) {
        d->status = kGeneralError;
        err = ENOMEM;
        goto zc_dev_act_is_sessioned_profile;
    }

    if ((err = kcpy(user, d->user, user_size)) != 0) {
        d->status = kGeneralError;
        goto zc_dev_act_is_sessioned_profile;
    }

    err = 0;

    profile = zacarias_profiles_ctx_get(g_cdev()->profiles, user, user_size);

    if (profile == NULL) {
        d->status = kProfileNotAttached;
        goto zc_dev_act_is_sessioned_profile;
    }

    d->sessioned = profile->sessioned;
    d->status = kNoError;

zc_dev_act_is_sessioned_profile:

    cdev_mtx_unlock(&g_cdev()->lock);

    profile = NULL;

    if (user != NULL) {
        kryptos_freeseg(user, user_size);
    }

    user_size = 0;

    return err;
}
*/

int zc_dev_act_attach_profile(struct zc_devio_ctx **devio) {
    struct zc_devio_ctx *d = *devio;
    int err = EFAULT;
    char *pwdb = NULL;
    size_t pwdb_size = 0;
    zacarias_profile_ctx *profile = NULL;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }

    if (d->pwdb_path == NULL || d->user == NULL) {
        ZC_DBG("null d->pwdb_path and/or d->user.\n");
        goto zc_dev_act_attach_profile_epilogue;
    }

    err = 0;

    if (zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size) != NULL) {
        d->status = kProfilePreviouslyAttached;
        ZC_DBG("profiled previously attached.\n");
        goto zc_dev_act_attach_profile_epilogue;
    }

    if (d->action == kAttachProfile) {
        if (kread(d->pwdb_path, &pwdb, &pwdb_size) != 0) {
            d->status = kPWDBReadingError;
            ZC_DBG("kread has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
    } else if (d->action == kInitAndAttachProfile) {
        pwdb_size = 8;
        pwdb = (char *) kryptos_newseg(pwdb_size);
        if (pwdb == NULL) {
            d->status = kGeneralError;
            ZC_DBG("unabled to allocate memory.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
        memset(pwdb, 0, pwdb_size);
    }

    if (zacarias_profiles_ctx_add(&g_cdev()->profiles, d->user, d->user_size, d->pwdb_path, d->pwdb_path_size, pwdb, pwdb_size) != 0) {
        d->status = kGeneralError;
        ZC_DBG("zacarias_profiles_ctx_add() has failed.\n");
        goto zc_dev_act_attach_profile_epilogue;
    }

    if ((profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size)) == NULL) {
        d->status = kProfileNotFound;
        ZC_DBG("zacarias_profiles_ctx_get() has failed.\n");
        goto zc_dev_act_attach_profile_epilogue;
    }

    if (d->action == kInitAndAttachProfile && profile != NULL) {
        if (plbuf_edit_add(&profile->plbuf, &profile->plbuf_size, "\x1BZ", 2, "\x1BZ", 2) != 0) {
            d->status = kPWDBWritingError;
            ZC_DBG("plbuf_edit_add() has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
        if (plbuf_edit_shuffle_stub(&profile->plbuf, &profile->plbuf_size) != 0) {
            err = 0;
            kryptos_freeseg(profile->plbuf, profile->plbuf_size);
            profile->plbuf = NULL;
            profile->plbuf_size = 0;
            d->status = kGeneralError;
            ZC_DBG("plbuf_edit_shuffle_stub() has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
        if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) == 0) {
            if (kwrite(profile->pwdb_path, profile->pwdb, profile->pwdb_size) != 0) {
                d->status = kPWDBWritingError;
                ZC_DBG("kwrite has failed.\n");
                goto zc_dev_act_attach_profile_epilogue;
            }
        } else {
            d->status = kPWDBWritingError;
            ZC_DBG("zacarias_encrypt_pwdb() has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
    } else if (d->action == kAttachProfile && profile != NULL) {
        if (zacarias_decrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
            d->status = kAuthenticationFailure;
            zacarias_profiles_ctx_del(&g_cdev()->profiles, d->user, d->user_size);
            ZC_DBG("zacarias_decrypt_pwdb() has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
        if (zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
            d->status = kGeneralError;
            zacarias_profiles_ctx_del(&g_cdev()->profiles, d->user, d->user_size);
            ZC_DBG("zacarias_encrypt_pwdb() has failed.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
    }

    if (d->sessioned) {
        // INFO(Rafael): The user asked for a session passwd. We need to patch pwdb in memory in order to deliver it.
        if (d->session_passwd_size == 0 || d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0) {
            err = EINVAL;
            d->status = kInvalidParams;
            ZC_DBG("invalid parameters.\n");
            goto zc_dev_act_attach_profile_epilogue;
        }
        profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size);
        if (zacarias_setkey_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size, d->session_passwd, d->session_passwd_size) == 0) {
            d->status = kNoError;
            profile->sessioned = 1;
        } else {
            ZC_DBG("zacarias_setkey_pwdb() has failed.\n");
            d->status = kAuthenticationFailure;
        }
    } else {
        d->status = kNoError;
    }

zc_dev_act_attach_profile_epilogue:

    if (pwdb != NULL) {
        kryptos_freeseg(pwdb, pwdb_size);
    }

    pwdb_size = 0;
    pwdb = NULL;

    profile = NULL;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_detach_profile(struct zc_devio_ctx **devio) {
    int err = EFAULT;
    struct zc_devio_ctx *d = *devio;
    zacarias_profile_ctx *profile;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        ZC_DBG("return EBUSY.\n");
        return EBUSY;
    }

    if (d->user == NULL || d->user_size == 0 ||
        d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0) {
        err = EINVAL;
        d->status = kInvalidParams;
        ZC_DBG("invalid parameters.\n");
        goto zc_dev_act_detach_profile_epilogue;
    }

    profile = zacarias_profiles_ctx_get(g_cdev()->profiles, d->user, d->user_size);
    if (profile == NULL) {
        err = 0;
        d->status = kProfileNotAttached;
        ZC_DBG("profile not found.\n");
        goto zc_dev_act_detach_profile_epilogue;
    }

    err = 0;

    if (zacarias_decrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size) != 0) {
        d->status = kAuthenticationFailure;
        ZC_DBG("zacarias_decrypt_pwdb() has failed.\n");
        goto zc_dev_act_detach_profile_epilogue;
    }

    // WARN(Rafael): Maybe it is so much paranoid. Maybe remove it in the future.
    zacarias_encrypt_pwdb(&profile, d->pwdb_passwd, d->pwdb_passwd_size);

    if (zacarias_profiles_ctx_del(&g_cdev()->profiles, d->user, d->user_size) != 0) {
        d->status = kGeneralError;
        ZC_DBG("zacarias_profiles_ctx_del() has failed.\n");
        goto zc_dev_act_detach_profile_epilogue;
    }

    d->status = kNoError;

zc_dev_act_detach_profile_epilogue:

    profile = NULL;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}
