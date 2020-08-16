#include <sec/plbuf_editor.h>
#include <string.h>

static const kryptos_u8_t *findstr(const kryptos_u8_t *haystack, const kryptos_u8_t *haystack_end,
                                   const kryptos_u8_t *needle, const kryptos_u8_t *needle_end);

int plbuf_edit_add(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size,
                   const kryptos_u8_t *passwd, const size_t passwd_size) {
    int err = 1;
    kryptos_u8_t *temp = NULL;
    size_t temp_size;
    kryptos_task_ctx t, *ktask = &t;

    kryptos_task_init_as_null(ktask);

    if (plbuf == NULL || plbuf_size == NULL || alias == NULL || alias_size == 0 ||
        passwd == NULL || passwd_size == 0) {
        goto plbuf_edit_add_epilogue;
    }

    if (findstr(*plbuf, (*plbuf) + *plbuf_size, alias, alias + alias_size) != NULL) {
        goto plbuf_edit_add_epilogue;
    }

    kryptos_task_set_encode_action(ktask);
    kryptos_run_encoder(base64, ktask, (kryptos_u8_t *)passwd, passwd_size);

    if (!kryptos_last_task_succeed(ktask)) {
        goto plbuf_edit_add_epilogue;
    }

    temp_size = *plbuf_size + alias_size + 4;
    temp = (kryptos_u8_t *) kryptos_newseg(temp_size + ktask->out_size);
    memset(temp, 0, temp_size);
    if ((*plbuf) != NULL) {
        memcpy(temp, *plbuf, *plbuf_size);
    }
    memcpy(temp + *plbuf_size, alias, alias_size);
    *(temp + *plbuf_size + alias_size) = '\t';
    memcpy(temp + *plbuf_size + alias_size + 1, ktask->out, ktask->out_size);
    *(temp + *plbuf_size + alias_size + ktask->out_size + 1) = '\n';

    err = 0;

plbuf_edit_add_epilogue:

    if (temp != NULL) {
        if (err == 0) {
            kryptos_freeseg(*plbuf, *plbuf_size);
            (*plbuf) = temp;
            *plbuf_size = temp_size;
        } else {
            kryptos_freeseg(temp, temp_size);
        }
        temp = NULL;
        temp_size = 0;
    }

    kryptos_task_free(ktask, KRYPTOS_TASK_OUT);

    return err;
}

int plbuf_edit_del(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size) {
    int err = 1;
    kryptos_u8_t *temp = NULL;
    size_t temp_size;
    const kryptos_u8_t *entry, *entry_end;
    kryptos_u8_t *pl, *pl_end;
    size_t pl_size;

    if (plbuf == NULL || plbuf_size == NULL || (*plbuf) != NULL || *plbuf_size == 0 || alias == NULL || alias_size == 0) {
        goto plbuf_edit_del_epilogue;
    }

    if ((entry = findstr(*plbuf, (*plbuf) + *plbuf_size, alias, alias + alias_size)) == NULL) {
        goto plbuf_edit_del_epilogue;
    }

    entry_end = entry;
    while (*entry_end != '\n') {
        entry_end++;
    }
    entry_end += 1;

    temp_size = *plbuf_size - (entry_end - entry);
    temp = (kryptos_u8_t *) kryptos_newseg(temp_size);

    if (temp == NULL) {
        goto plbuf_edit_del_epilogue;
    }

    memset(temp, 0, temp_size);

    entry_end += 1;

    if (entry == (*plbuf)) {
        if (entry_end < (*plbuf) + *plbuf_size) {
            memcpy(temp, entry_end, temp_size);
        }
    } else {
        pl = (*plbuf);
        pl_end = pl;

        while (pl_end != entry) {
            pl_end++;
        }

        pl_size = pl_end - pl;
        memcpy(temp, pl, pl_size);
        if (entry_end < (*plbuf) + *plbuf_size) {
            pl_size = *plbuf_size - pl_size - (entry_end - entry);
            memcpy(temp + pl_size, entry_end, pl_size);
        }
    }

    err = 0;

plbuf_edit_del_epilogue:

    if (temp != NULL) {
        if (err == 0) {
            kryptos_freeseg((*plbuf), *plbuf_size);
            (*plbuf) = temp;
            *plbuf_size = temp_size;
        } else {
            kryptos_freeseg(temp, temp_size);
        }
        temp_size = 0;
        temp = NULL;
    }

    entry_end = entry = pl_end = pl = NULL;
    pl_size = 0;

    return err;
}

int plbuf_edit_find(const kryptos_u8_t *plbuf, const size_t plbuf_size,
                    const kryptos_u8_t *alias, const size_t alias_size) {
    return (findstr(plbuf, plbuf + plbuf_size, alias, alias + alias_size) != NULL);
}

static const kryptos_u8_t *findstr(const kryptos_u8_t *haystack, const kryptos_u8_t *haystack_end,
                                   const kryptos_u8_t *needle, const kryptos_u8_t *needle_end) {
    const kryptos_u8_t *hp;
    size_t needle_size;

    if (haystack == NULL || haystack_end == NULL || needle == NULL || needle_end == NULL) {
        return NULL;
    }

    hp = haystack;
    needle_size = needle_end - needle;

    while (hp != haystack_end && (haystack_end - hp) >= needle_size) {
        if (memcmp(hp, needle, needle_size) == 0) {
            return hp;
        }
        hp++;
    }

    return NULL;
}
