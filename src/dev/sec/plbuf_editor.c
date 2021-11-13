/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <sec/plbuf_editor.h>
#include <sec/crypto.h>
#ifndef KRYPTOS_KERNEL_MODE
# include <string.h>
#endif

static const kryptos_u8_t *findalias(const kryptos_u8_t *haystack, const kryptos_u8_t *haystack_end,
                                     const kryptos_u8_t *needle, const kryptos_u8_t *needle_end);

static kryptos_u8_t *random_plbuf_entry(size_t *size);

kryptos_u8_t *plbuf_edit_aliases(const kryptos_u8_t *plbuf, const size_t plbuf_size,
                                 size_t *aliases_size) {
    const kryptos_u8_t *p = NULL, *p_end = NULL;
    kryptos_u8_t *aliases = NULL, *ap = NULL, *ap_end = NULL;

    if (plbuf == NULL || plbuf_size == 0 || aliases_size == NULL) {
        goto plbuf_edit_aliases_epilogue;
    }

    p = plbuf;
    p_end = p + plbuf_size;

    *aliases_size = 0;

    while (p != p_end) {
        p++;
        if (*p != '\t') {
            (*aliases_size)++;
        } else {
            do {
                p++;
            } while (p != p_end && *p != '\n');
        }
    }

    if (*aliases_size == 0) {
        goto plbuf_edit_aliases_epilogue;
    }

    aliases = (kryptos_u8_t *) kryptos_newseg(*aliases_size + 1);

    if (aliases == NULL) {
        *aliases_size = 0;
        goto plbuf_edit_aliases_epilogue;
    }

    memset(aliases, 0, *aliases_size + 1);
    ap = aliases;
    ap_end = ap + *aliases_size;

    p = plbuf;

    while (p != p_end && ap != ap_end) {
        if (*p != '\t') {
            *ap = *p;
        } else {
            ap++;
            if (ap == ap_end) {
                // INFO(Rafael): It should never happen in normal conditions.
                continue;
            }
            *ap = '\n';
            do {
                p++;
            } while (p != p_end && *p != '\n');
        }
        p++;
        ap++;
    }

plbuf_edit_aliases_epilogue:

    p = p_end = NULL;
    ap = ap_end = NULL;

    return aliases;
}

kryptos_u8_t *plbuf_edit_passwd(const kryptos_u8_t *plbuf, const size_t plbuf_size,
                                const kryptos_u8_t *alias, const size_t alias_size,
                                size_t *passwd_size) {
    kryptos_task_ctx t, *ktask = &t;
    kryptos_u8_t *passwd = NULL;
    const kryptos_u8_t *p = NULL, *p_end = NULL, *end = NULL;
    kryptos_u8_t *buf = NULL;
    size_t buf_size = 0;

    kryptos_task_init_as_null(ktask);

    if (plbuf == NULL || plbuf_size == 0 || alias == NULL || alias_size == 0 || passwd_size == NULL) {
        goto plbuf_edit_passwd_epilogue;
    }

    end = plbuf + plbuf_size;

    p = findalias(plbuf, end, alias, alias + alias_size);

    if (p == NULL) {
        goto plbuf_edit_passwd_epilogue;
    }

    while (p != end && *p != '\t') {
        p++;
    }

    if (p == end) {
        goto plbuf_edit_passwd_epilogue;
    }

    p += 1;

    p_end = p;
    while (p_end != end && *p_end != '\n') {
        p_end++;
    }

    buf_size = p_end - p;
    if ((buf = (kryptos_u8_t *) kryptos_newseg(buf_size + 1)) == NULL) {
        goto plbuf_edit_passwd_epilogue;
    }
    memset(buf, 0, buf_size + 1);
    memcpy(buf, p, buf_size);

    kryptos_task_set_decode_action(ktask);
    kryptos_run_encoder(base64, ktask, buf, buf_size);

plbuf_edit_passwd_epilogue:

    if (kryptos_last_task_succeed(ktask)) {
        passwd = ktask->out;
        *passwd_size = ktask->out_size;
    }

    if (buf != NULL) {
        kryptos_freeseg(buf, buf_size);
        buf_size = 0;
    }

    kryptos_task_init_as_null(ktask);

    p = p_end = end = NULL;
    buf = NULL;
    buf_size = 0;

    return passwd;
}

int plbuf_edit_detach(kryptos_u8_t **plbuf, size_t *plbuf_size) {
    int err = 1;
    kryptos_u8_t *p = NULL, *p_end = NULL;
    kryptos_u8_t *temp = NULL;
    size_t temp_size = 0;

    if (plbuf == NULL || (*plbuf) == NULL || plbuf_size == NULL || *plbuf_size == 0) {
        goto plbuf_edit_detach_epilogue;
    }

    p = (*plbuf);
    p_end = p + *plbuf_size;

    while (p != p_end && *p != '\n') {
        p++;
    }

    if (*p != '\n') {
        goto plbuf_edit_detach_epilogue;
    }

    p += 1;

    p_end -= 1;

    p_end -= (*p_end == '\n');

    while (p_end != p && *p_end != '\n') {
        p_end--;
    }

    if (*p == 0x1B) {
        kryptos_freeseg((*plbuf), *plbuf_size);
        (*plbuf) = NULL;
        *plbuf_size = 0;
        err = 0;
        goto plbuf_edit_detach_epilogue;
    }

    p_end += (*p_end == '\n');
    temp_size = p_end - p;
    if ((temp = (kryptos_u8_t *) kryptos_newseg(temp_size + 1)) == NULL) {
        goto plbuf_edit_detach_epilogue;
    }

    memset(temp, 0, temp_size + 1);
    memcpy(temp, p, temp_size);

    err = 0;

plbuf_edit_detach_epilogue:

    if (temp != NULL && err == 0 && (*plbuf) != NULL) {
        kryptos_freeseg((*plbuf), *plbuf_size);
        (*plbuf) = temp;
        *plbuf_size = temp_size;
        temp = NULL;
        temp_size = 0;
    } else if (temp != NULL) {
        kryptos_freeseg(temp, temp_size);
        temp = NULL;
        temp_size = 0;
    }

    p = p_end = NULL;
    temp_size = 0;

    return err;
}

int plbuf_edit_shuffle_stub(kryptos_u8_t **plbuf, size_t *plbuf_size) {
    kryptos_u8_t *random_entries[2] = { NULL, NULL };
    size_t random_entries_size[2] = { 0, 0 };
    int err = 1;
    kryptos_u8_t *stub = NULL;
    size_t stub_size = 0;

    if (plbuf == NULL || *plbuf == NULL || plbuf_size == NULL || *plbuf_size == 0) {
        goto plbuf_edit_shuffle_stub_epilogue;
    }

    random_entries[0] = random_plbuf_entry(&random_entries_size[0]);
    if (random_entries[0] == NULL) {
        goto plbuf_edit_shuffle_stub_epilogue;
    }

    random_entries[1] = random_plbuf_entry(&random_entries_size[1]);
    if (random_entries[1] == NULL) {
        goto plbuf_edit_shuffle_stub_epilogue;
    }

    stub_size = *plbuf_size + random_entries_size[0] + random_entries_size[1];
    stub = kryptos_newseg(stub_size + 1);
    if (stub == NULL) {
        goto plbuf_edit_shuffle_stub_epilogue;
    }

    memset(stub, 0, stub_size + 1);
    memcpy(stub, random_entries[0], random_entries_size[0]);
    memcpy(stub + random_entries_size[0], *plbuf, *plbuf_size);
    memcpy(stub + random_entries_size[0] + *plbuf_size, random_entries[1], random_entries_size[1]);

    kryptos_freeseg(*plbuf, *plbuf_size);
    *plbuf = stub;
    *plbuf_size = stub_size;
    stub_size = 0;
    stub = NULL;
    err = 0;

plbuf_edit_shuffle_stub_epilogue:

    if (random_entries[0] != NULL) {
        kryptos_freeseg(random_entries[0], random_entries_size[0]);
        random_entries[0] = NULL;
        random_entries_size[0] = 0;
    }

    if (random_entries[1] != NULL) {
        kryptos_freeseg(random_entries[1], random_entries_size[1]);
        random_entries[1] = NULL;
        random_entries_size[1] = 0;
    }

    if (stub != NULL) {
        kryptos_freeseg(stub, stub_size);
        stub = NULL;
        stub_size = 0;
    }

    return err;
}

int plbuf_edit_shuffle(kryptos_u8_t **plbuf, size_t *plbuf_size) {
    int err = 1;
    kryptos_u8_t **plbuf_lines = NULL;
    size_t plbuf_lines_nr = 0, plbuf_n = 0;
    kryptos_u8_t *p = NULL, *p_end = NULL, *master_end = NULL;
    int done = 0;
    kryptos_u8_t *temp = NULL, *tp = NULL;
    size_t temp_size = 0;
    kryptos_u8_t *random_entries[2];
    size_t random_entries_size[2];

    random_entries[0] = random_entries[1] = NULL;
    random_entries_size[0] = random_entries_size[1] = 0;

    if (plbuf == NULL || *plbuf == NULL || plbuf_size == NULL || *plbuf_size == 0) {
        goto plbuf_edit_shuffle_epilogue;
    }

    random_entries[0] = random_plbuf_entry(&random_entries_size[0]);
    random_entries[1] = random_plbuf_entry(&random_entries_size[1]);

    if (random_entries[0] == NULL || random_entries[1] == NULL) {
        goto plbuf_edit_shuffle_epilogue;
    }

    plbuf_lines_nr = 0;

    p = (*plbuf);
    master_end = p + *plbuf_size;
    p_end = master_end;

    while (p != p_end) {
        if (*p == '\n') {
            plbuf_lines_nr += 1;
        }
        p++;
    }

    plbuf_lines = (kryptos_u8_t **) kryptos_newseg(sizeof(kryptos_u8_t *) * plbuf_lines_nr);

    if (plbuf_lines == NULL) {
        goto plbuf_edit_shuffle_epilogue;
    }

    plbuf_n = 0;

    p = (*plbuf);

    if (*p != 0x1B) {
        plbuf_lines[plbuf_n++] = p++;
        while (p != p_end) {
            if (*p == '\n' && p + 1 < p_end && p + 1 < master_end && *(p + 1) != '\n') {
                plbuf_lines[plbuf_n++] = p + 1;
            }
            p++;
        }
    }

    done = (plbuf_n == 0);

    if (done) {
        kryptos_freeseg(*plbuf, *plbuf_size);
        *plbuf = NULL;
        *plbuf_size = 0;
        err = 0;
        goto plbuf_edit_shuffle_epilogue;
    }

    temp_size = *plbuf_size + random_entries_size[0] + random_entries_size[1];
    temp = (kryptos_u8_t *) kryptos_newseg(temp_size + 1);
    memset(temp, 0, temp_size + 1);

    memcpy(temp, random_entries[0], random_entries_size[0]);
    tp = temp + random_entries_size[0];

    plbuf_n = 0;

#define random_plbuf_line_n ( ( (size_t) kryptos_get_random_byte() << 24 |\
                                (size_t) kryptos_get_random_byte() << 16 |\
                                (size_t) kryptos_get_random_byte() <<  8 |\
                                (size_t) kryptos_get_random_byte() ) % plbuf_lines_nr )

    while (!done) {
        do {
            plbuf_n = random_plbuf_line_n;
        } while (plbuf_lines[plbuf_n] == NULL);

        p = plbuf_lines[plbuf_n];

        p_end = p;
        while (p_end < master_end && *p_end != '\n') {
            p_end++;
        }
        if (p_end == master_end) {
            // WARN(Rafael): It seems screwed up. It is better not going ahead with this buffer.
            goto plbuf_edit_shuffle_epilogue;
        }
        p_end += 1;

        memcpy(tp, p, p_end - p);
        tp += p_end - p;

        plbuf_lines[plbuf_n] = NULL;

        done = 1;
        for (plbuf_n = 0; plbuf_n < plbuf_lines_nr && done; plbuf_n++) {
            done = (plbuf_lines[plbuf_n] == NULL);
        }
    }

#undef random_plbuf_line_n

    memcpy(tp, random_entries[1], random_entries_size[1]);

    err = 0;

plbuf_edit_shuffle_epilogue:

    if (temp != NULL && err == 0) {
        kryptos_freeseg(*plbuf, *plbuf_size);
        (*plbuf) = temp;
        (*plbuf_size) = temp_size;
        temp = NULL;
        temp_size = 0;
    } else if (temp != NULL) {
        kryptos_freeseg(temp, temp_size);
        temp = NULL;
        temp_size = 0;
    }

    if (plbuf_lines != NULL) {
        kryptos_freeseg(plbuf_lines, sizeof(kryptos_u8_t *) * plbuf_lines_nr);
        plbuf_lines_nr = 0;
    }

    if (random_entries[0] != NULL) {
        kryptos_freeseg(random_entries[0], random_entries_size[0]);
        random_entries[0] = NULL;
        random_entries_size[0] = 0;
    }

    if (random_entries[1] != NULL) {
        kryptos_freeseg(random_entries[1], random_entries_size[1]);
        random_entries[1] = NULL;
        random_entries_size[1] = 0;
    }

    plbuf_lines = NULL;

    tp = p = p_end = master_end = NULL;
    done = 0;
    plbuf_n = 0;
    plbuf_lines_nr = 0;

    return err;
}

int plbuf_edit_add(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size,
                   const kryptos_u8_t *passwd, const size_t passwd_size) {
#if defined(__FreeBSD__) && defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif
    int err = 1;
    kryptos_u8_t *temp = NULL;
    size_t temp_size = 0;
    kryptos_task_ctx t, *ktask = &t;

    kryptos_task_init_as_null(ktask);

    if (plbuf == NULL || plbuf_size == NULL || alias == NULL || alias_size == 0 ||
        passwd == NULL || passwd_size == 0) {
        goto plbuf_edit_add_epilogue;
    }

    if (findalias(*plbuf, (*plbuf) + *plbuf_size, alias, alias + alias_size) != NULL) {
        goto plbuf_edit_add_epilogue;
    }

    kryptos_task_set_encode_action(ktask);
    kryptos_run_encoder(base64, ktask, (kryptos_u8_t *)passwd, passwd_size);

    if (!kryptos_last_task_succeed(ktask)) {
        goto plbuf_edit_add_epilogue;
    }

    temp_size = *plbuf_size + alias_size + ktask->out_size + 2;
    if ((temp = (kryptos_u8_t *) kryptos_newseg(temp_size + 1)) == NULL) {
        goto plbuf_edit_add_epilogue;
    }
    memset(temp, 0, temp_size + 1);

    if ((*plbuf) != NULL) {
        memcpy(temp, *plbuf, *plbuf_size);
    } else {
        *plbuf_size = 0;
    }

    memcpy(temp + *plbuf_size, alias, alias_size);
    *(temp + *plbuf_size + alias_size) = '\t';
    memcpy(temp + *plbuf_size + alias_size + 1, ktask->out, ktask->out_size);
    *(temp + *plbuf_size + alias_size + ktask->out_size  + 1) = '\n';

    err = 0;

plbuf_edit_add_epilogue:

    if (temp != NULL) {
        if (err == 0) {
            if ((*plbuf) != NULL) {
                kryptos_freeseg(*plbuf, *plbuf_size);
            }
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

#if defined(__FreeBSD___) && defined(__clang__)
# pragma pop
#endif
}

int plbuf_edit_del(kryptos_u8_t **plbuf, size_t *plbuf_size,
                   const kryptos_u8_t *alias, const size_t alias_size) {
    int err = 1;
    kryptos_u8_t *temp = NULL;
    size_t temp_size = 0;
    const kryptos_u8_t *entry, *entry_end;
    kryptos_u8_t *pl = NULL, *pl_end = NULL;
    size_t pl_size = 0;

    if (plbuf == NULL || plbuf_size == NULL || (*plbuf) == NULL || *plbuf_size == 0 || alias == NULL || alias_size == 0) {
        goto plbuf_edit_del_epilogue;
    }

    if ((entry = findalias(*plbuf, (*plbuf) + *plbuf_size, alias, alias + alias_size)) == NULL) {
        goto plbuf_edit_del_epilogue;
    }

    entry_end = entry;
    while (*entry_end != '\n') {
        entry_end++;
    }
    entry_end += 1;

    temp_size = *plbuf_size - (entry_end - entry);
    temp = (kryptos_u8_t *) kryptos_newseg(temp_size + 1);

    if (temp == NULL) {
        goto plbuf_edit_del_epilogue;
    }

    memset(temp, 0, temp_size + 1);

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
            memcpy(temp + pl_size, entry_end, *plbuf_size - pl_size - (entry_end - entry));
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
    if (alias_size == 0 || plbuf_size == 0) {
        return 0;
    }
    return (findalias(plbuf, plbuf + plbuf_size, alias, alias + alias_size) != NULL);
}

static const kryptos_u8_t *findalias(const kryptos_u8_t *haystack, const kryptos_u8_t *haystack_end,
                                     const kryptos_u8_t *needle, const kryptos_u8_t *needle_end) {
    const kryptos_u8_t *hp;
    size_t needle_size;

    if (haystack == NULL || haystack_end == NULL || needle == NULL || needle_end == NULL) {
        return NULL;
    }

    hp = haystack;
    needle_size = needle_end - needle;

    while (hp < haystack_end && (size_t)(haystack_end - hp) >= needle_size) {
        if (memcmp(hp, needle, needle_size) == 0) {
            return hp;
        }
        while (hp != haystack_end && *hp != '\n') {
            hp++;
        }
        hp += (*hp == '\n');
    }

    return NULL;
}

static kryptos_u8_t *random_plbuf_entry(size_t *size) {
    static kryptos_u8_t *randcharset = (kryptos_u8_t *)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789=/<>.,%#@!*(){}[]+-\\";
    static size_t randcharset_size = 82;
    kryptos_u8_t *entry, *ep, *ep_end;

    if (size == NULL) {
        return NULL;
    }

    *size = 0;
    while (*size == 0) {
        *size = kryptos_get_random_byte();
    }

    if ((entry = (kryptos_u8_t *) kryptos_newseg(*size + 2)) == NULL) {
        return NULL;
    }

    memset(entry, 0, *size + 2);

    ep = entry;
    ep_end = ep + *size - 1;

    while (ep != ep_end) {
        *ep = randcharset[kryptos_unbiased_rand_mod_u8(randcharset_size)];
        ep++;
    }

    ep_end[0] = '\n';

    return entry;
}
