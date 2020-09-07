#include <defs/io.h>
#include <defs/types.h>
#include <linux/slab.h>
#include <ctx/ctx.h>

int zc_dev_act_attach_profile(struct zc_devio_ctx **devio) {
    char *pwdb_path = NULL;
    size_t pwdb_path_size;
    char *user = NULL;
    size_t user_size;
    struct zc_devio_ctx *d = *devio;
    int err = -EFAULT;
    char *pwdb = NULL;
    size_t pwdb_size;

    if (!cdev_mtx_lock(&g_cdev()->lock)) {
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

    if (zacarias_profiles_ctx_get(g_cdev()->profiles, user, user_size) != NULL) {
        d->status = kProfilePreviouslyAttached;
        goto zc_dev_act_attach_profile_epilogue;
    }

    if ((err = kread(pwdb_path, &pwdb, &pwdb_size)) != 0) {
        d->status = kPWDBReadingError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    err = zacarias_profiles_ctx_add(g_cdev()->profiles, user, user_size, pwdb, pwdb_size);
    if (err != 0) {
        err = -err;
        d->status = kGeneralError;
        goto zc_dev_act_attach_profile_epilogue;
    }

    user = NULL;
    pwdb = NULL;

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

    user_size = pwdb_path_size = pwdb_size = 0;

    cdev_unlock(&g_cdev()->lock);

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
