#include <defs/io.h>
#include <defs/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <kio.h>
#include <ctx/ctx.h>
#include <sec/crypto.h>

int zc_dev_act_setkey(struct zc_devio_ctx **devio) {
    int err = -EFAULT;
    struct zc_devio_ctx *d = *devio;
    char *user = NULL;
    size_t user_size;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size;
    unsigned char *new_passwd = NULL;
    size_t new_passwd_size;
    zacarias_profile_ctx *profile;

    if (cdev_mtx_trylock(&g_cdev()->lock)) {
        return -EBUSY;
    }

    if (d->user == NULL || d->user_size == 0 || d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0 ||
        d->passwd == NULL || d->passwd_size == 0) {
        err = -EINVAL;
        d->status = kInvalidParams;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    user_size = d->user_size;
    user = (char *) kmalloc(user_size, GFP_ATOMIC);
    if (user == NULL) {
        err = -ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    pwdb_passwd_size = d->pwdb_passwd_size;
    pwdb_passwd = (unsigned char *) kmalloc(pwdb_passwd_size, GFP_ATOMIC);
    if (pwdb_passwd == NULL) {
        err = -ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_setkey_profile_epilogue;
    }

    new_passwd_size = d->passwd_size;
    new_passwd = (unsigned char *) kmalloc(new_passwd_size, GFP_ATOMIC);
    if (new_passwd == NULL) {
        err = -ENOMEM;
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
        kfree(user);
    }

    if (pwdb_passwd != NULL) {
        memset(pwdb_passwd, 0, pwdb_passwd_size);
        kfree(pwdb_passwd);
    }

    if (new_passwd != NULL) {
        memset(new_passwd, 0, new_passwd_size);
        kfree(new_passwd);
    }

    user_size = pwdb_passwd_size = new_passwd_size = 0;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_is_sessioned_profile(struct zc_devio_ctx **devio) {
    int err = -EFAULT;
    char *user = NULL;
    size_t user_size;
    struct zc_devio_ctx *d = *devio;
    zacarias_profile_ctx *profile = NULL;

    if (cdev_mtx_trylock(&g_cdev()->lock)) {
        return -EBUSY;
    }

    if (d->user == NULL || d->user_size == 0) {
        d->status = kInvalidParams;
        err = -EINVAL;
        goto zc_dev_act_is_sessioned_profile;
    }

    user_size = d->user_size;
    user = (char *) kmalloc(user_size, GFP_ATOMIC);
    if (user == NULL) {
        d->status = kGeneralError;
        err = -ENOMEM;
        goto zc_dev_act_is_sessioned_profile;
    }

    if ((err = copy_from_user(user, d->user, user_size)) != 0) {
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
        kfree(user);
    }

    return err;
}

int zc_dev_act_attach_profile(struct zc_devio_ctx **devio) {
    char *pwdb_path = NULL;
    size_t pwdb_path_size;
    char *user = NULL;
    size_t user_size;
    struct zc_devio_ctx *d = *devio;
    int err = -EFAULT;
    char *pwdb = NULL;
    size_t pwdb_size;
    unsigned char *session_passwd = NULL;
    size_t session_passwd_size;
    zacarias_profile_ctx *profile = NULL;
    unsigned char *pwdb_passwd = NULL;
    size_t pwdb_passwd_size;

    if (!cdev_mtx_trylock(&g_cdev()->lock)) {
        return -EBUSY;
    }

    if (d->pwdb_path == NULL || d->user == NULL) {
        goto zc_dev_act_attach_profile_epilogue;
    }

    pwdb_path_size = d->pwdb_path_size;
    pwdb_path = (char *) kmalloc(pwdb_path_size + 1, GFP_ATOMIC);
    if (pwdb_path == NULL) {
        err = -ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_attach_profile_epilogue; 
    }
    memset(pwdb_path, 0, pwdb_path_size + 1);

    if ((err = copy_from_user(pwdb_path, d->pwdb_path, pwdb_path_size)) != 0) {
        goto zc_dev_act_attach_profile_epilogue;
    }

    user_size = d->user_size;
    user = (char *) kmalloc(user_size, GFP_ATOMIC);
    if (user == NULL) {
        err = -ENOMEM;
        d->status = kGeneralError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    if ((err = copy_from_user(user, d->user, user_size)) != 0) {
        d->status = kGeneralError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    err = 0;

    if (zacarias_profiles_ctx_get(g_cdev()->profiles, user, user_size) != NULL) {
        d->status = kProfilePreviouslyAttached;
        goto zc_dev_act_attach_profile_epilogue;
    }

    if (kread(pwdb_path, &pwdb, &pwdb_size) != 0) {
        d->status = kPWDBReadingError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    if (zacarias_profiles_ctx_add(&g_cdev()->profiles, user, user_size, pwdb_path, pwdb_path_size, pwdb, pwdb_size) != 0) {
        d->status = kGeneralError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    user = NULL;
    pwdb_path = NULL;
    pwdb = NULL;

    if (d->session_passwd != NULL) {
        // INFO(Rafael): The user asked for a session passwd. We need to patch pwdb in memory in order to deliver it.
        if (d->session_passwd_size == 0 || d->pwdb_passwd == NULL || d->pwdb_passwd_size == 0) {
            err = -EINVAL;
            d->status = kInvalidParams;
            goto zc_dev_act_attach_profile_epilogue;
        }

        session_passwd_size = d->session_passwd_size;
        session_passwd = (unsigned char *)kmalloc(session_passwd_size, GFP_ATOMIC);
        if (session_passwd == NULL) {
            err = -ENOMEM;
            d->status = kGeneralError;
            goto zc_dev_act_attach_profile_epilogue;
        }

        pwdb_passwd_size = d->pwdb_passwd_size;
        pwdb_passwd = (unsigned char *)kmalloc(pwdb_passwd_size, GFP_ATOMIC);
        if (pwdb_passwd == NULL) {
            err = -ENOMEM;
            d->status = kGeneralError;
            goto zc_dev_act_attach_profile_epilogue;
        }

        if (copy_from_user(session_passwd, d->session_passwd, session_passwd_size) != 0) {
            d->status = kGeneralError;
            goto zc_dev_act_attach_profile_epilogue;
        }

        if (copy_from_user(pwdb_passwd, d->pwdb_passwd, pwdb_passwd_size) != 0) {
            d->status = kGeneralError;
            goto zc_dev_act_attach_profile_epilogue;
        }

        profile = zacarias_profiles_ctx_get(g_cdev()->profiles, user, user_size);
        if (zacarias_setkey_pwdb(&profile, pwdb_passwd, pwdb_passwd_size, session_passwd, session_passwd_size) == 0) {
            d->status = kNoError;
            profile->sessioned = 1;
        } else {
            d->status = kAuthenticationFailure;
        }

        memset(session_passwd, 0, session_passwd_size);
        memset(pwdb_passwd, 0, pwdb_passwd_size);
    } else {
        d->status = kNoError;
    }

zc_dev_act_attach_profile_epilogue:

    if (pwdb_path != NULL) {
        kfree(pwdb_path);
    }

    if (user != NULL) {
        kfree(user);
    }

    if (pwdb != NULL) {
        kfree(pwdb);
    }

    if (session_passwd != NULL) {
        kfree(session_passwd);
    }

    if (pwdb_passwd != NULL) {
        kfree(pwdb_passwd);
    }

    user_size = pwdb_path_size = pwdb_size = session_passwd_size = pwdb_passwd_size = 0;

    profile = NULL;

    cdev_mtx_unlock(&g_cdev()->lock);

    return err;
}

int zc_dev_act_detach_profile(struct zc_devio_ctx **devio) {
    // TODO(Rafael): Guess what?
    return -EFAULT;
}

int zc_dev_act_add_password(struct zc_devio_ctx **devio) {
    // TODO(Rafael): Guess what?
    return -EFAULT;
}

int zc_dev_act_del_password(struct zc_devio_ctx **devio) {
    // TODO(Rafael): Guess what?
    return -EFAULT;
}

int zc_dev_act_get_password(struct zc_devio_ctx **devio) {
    // TODO(Rafael): Guess what?
    return -EFAULT;
}
