/*
 *                          Copyright (C) 2020 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <kbd/kbd.h>
#include <ctype.h>
#include <stdio.h>
#if !defined(_WIN32)
# include <termios.h>
#else
# include <windows.h>
#endif
#include <signal.h>
#include <unistd.h>
#include <kbd/kmap.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#if !defined(_WIN32)
static struct termios old, new;
#else
static DWORD con_mode;
#endif

static void getuserkey_sigint_watchdog(int signo);

int zacarias_sendkeys(const kryptos_u8_t *buffer, const size_t buffer_size, const unsigned int timeout_in_secs) {
    kryptos_u8_t *input = NULL, *ip = NULL, *ip_end = NULL;
    size_t input_size = 0;
    int err = 0, hold_sh = 0;
    unsigned int keycode = 0, shiftcode = 0;
    Display *display = NULL;
    kryptos_u8_t abs_key = 0;

    if (buffer == NULL || buffer_size == 0) {
        return 1;
    }

    if ((display = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "error: Unable to open X display.\n");
        err = 1;
        goto zacarias_sendkeys_epilogue;
    }

    if (gZacariasCurrKbdLayout->key_demuxer != NULL) {
        input = gZacariasCurrKbdLayout->key_demuxer(buffer, buffer_size, &input_size);
    } else {
        input = (kryptos_u8_t *)buffer;
        input_size = buffer_size;
    }

    shiftcode = XKeysymToKeycode(display, (KeySym)XK_Shift_L);

    ip = input;
    ip_end = input + input_size;

    sleep(timeout_in_secs);

    while (ip != ip_end) {
        abs_key = gZacariasCurrKbdLayout->lower[*ip];
        if (abs_key == 0) {
            abs_key = gZacariasCurrKbdLayout->upper[*ip];
            hold_sh = 1;
        }

        if (abs_key == 0) {
            fprintf(stderr, "error: Unsupported symbol. Aborted. Did you specified the right keyboard location?\n");
            err = 1;
            goto zacarias_sendkeys_epilogue;
        }

        keycode = XKeysymToKeycode(display, (KeySym)*input);

        if (hold_sh) {
            XTestFakeKeyEvent(display, shiftcode, 1, 0);
        }

        XTestFakeKeyEvent(display, keycode, 1, 0);
        XTestFakeKeyEvent(display, keycode, 0, 0);

        if (hold_sh) {
            XTestFakeKeyEvent(display, shiftcode, 0, 0);
            hold_sh = 0;
        }

        ip++;
    }

zacarias_sendkeys_epilogue:

    if (display != NULL) {
        XFlush(display);
        XCloseDisplay(display);
        display = NULL;
    }

    if (input != buffer) {
        kryptos_freeseg(input, input_size); // INFO(Rafael): kryptos_freeseg will clean up the buffer before freeing it.
    }

    // INFO(Rafael): Making sure that we will not leak any information about the processing state.

    input_size = 0;

    abs_key = 0;

    keycode = shiftcode = 0;
    hold_sh = 0;

    ip = ip_end = NULL;

    return err;
}

int zacarias_set_kbd_layout(const char *name) {
    zacarias_kmap_t *kp = &gZacariasKmap[0], *kp_end = &gZacariasKmap[0] + gZacariasKmapNr;

    while (kp != kp_end) {
        if (strcmp(kp->layout.name, name) == 0) {
            gZacariasCurrKbdLayout = &kp->layout;
            return 1;
        }
    }

    return 0;
}

#if defined(_WIN32)

#define stty_echo_off system("stty -echo");

#define stty_echo_on system("stty echo");

static int is_toynix(void);
#endif

static void getuserkey_sigint_watchdog(int signo) {
#if !defined(_WIN32)
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &old);
#else
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), con_mode);
    if (is_toynix()) {
        stty_echo_on
    }
#endif
    exit(1);
}

#if !defined(_WIN32)

kryptos_u8_t *zacarias_getuserkey(size_t *key_size) {
    kryptos_u8_t *key = NULL, *kp;
    char line[65535], *lp, *lp_end;
    size_t size;

    if (key_size == NULL || tcgetattr(STDOUT_FILENO, &old) != 0) {
        return NULL;
    }

    *key_size = 0;

    memcpy(&new, &old, sizeof(new));
    new.c_lflag &= ~ECHO;
    if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &new) != 0) {
        goto zacarias_getuserkey_epilogue;
    }

    signal(SIGINT, getuserkey_sigint_watchdog);
    signal(SIGTERM, getuserkey_sigint_watchdog);

    fgets(line, sizeof(line), stdin);
    //fprintf(stdout, "\n");

    size = strlen(line) - 1;

    if (size == 0) {
        goto zacarias_getuserkey_epilogue;
    }

    key = (kryptos_u8_t *) kryptos_newseg(size);
    kp = key;
    lp = &line[0];
    lp_end = lp + size;

    while (lp < lp_end) {
        if (*lp == '\\') {
            lp += 1;
            switch (*lp) {
                case 'x':
                    if ((lp + 2) < lp_end && isxdigit(lp[1]) && isxdigit(lp[2])) {
#define getnibble(b) ( isdigit((b)) ? ( (b) - '0' ) : ( toupper((b)) - 55 ) )
                        *kp = getnibble(lp[1]) << 4 | getnibble(lp[2]);
#undef getnibble
                        lp += 2;
                    } else {
                        *kp = *lp;
                    }
                    break;

                case 'n':
                    *kp = '\n';
                    break;

                case 't':
                    *kp = '\t';
                    break;

                default:
                    *kp = *lp;
                    break;
            }
        } else {
            *kp = *lp;
        }

        lp++;
        kp++;
    }

    *key_size = kp - key;

zacarias_getuserkey_epilogue:

    memset(line, 0, sizeof(line));

    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &old);

    return key;
}

#else

static int is_toynix(void) {
    static int is = -1;
    char data[256];
    if (is == -1) {
        is = (getenv("MSYSTEM") != NULL);
    }
    return is;
}

kryptos_u8_t *zacarias_getuserkey(size_t *key_size) {
    kryptos_u8_t *key = NULL, *kp;
    char line[65535], *lp, *lp_end;
    size_t size;

    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &con_mode);

    if (key_size == NULL) {
        return NULL;
    }

    *key_size = 0;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // INFO(Rafael): It is important disable ECHO_INPUT through SetConsoleMode inconditionaly, not based on !
    //               MSYSTEM environment variable definition. Otherwise will be possible echo the password  !
    //               at screen in command prompt just by defining a dummy MSYSTEM variable.                 !
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), con_mode & (~ENABLE_ECHO_INPUT));

    if (is_toynix()) {
        stty_echo_off
    }

    signal(SIGINT, getuserkey_sigint_watchdog);
    signal(SIGTERM, getuserkey_sigint_watchdog);

    fgets(line, sizeof(line), stdin);
    //fprintf(stdout, "\n");

    size = strlen(line) - 1;

    if (size == 0) {
        goto zacarias_getuserkey_epilogue;
    }

    key = (kryptos_u8_t *) kryptos_newseg(size);
    kp = key;
    lp = &line[0];
    lp_end = lp + size;

    while (lp < lp_end) {
        if (*lp == '\\') {
            lp += 1;
            switch (*lp) {
                case 'x':
                    if ((lp + 2) < lp_end && isxdigit(lp[1]) && isxdigit(lp[2])) {
#define getnibble(b) ( isdigit((b)) ? ( (b) - '0' ) : ( toupper((b)) - 55 ) )
                        *kp = getnibble(lp[1]) << 4 | getnibble(lp[2]);
#undef getnibble
                        lp += 2;
                    } else {
                        *kp = *lp;
                    }
                    break;

                case 'n':
                    *kp = '\n';
                    break;

                case 't':
                    *kp = '\t';
                    break;

                default:
                    *kp = *lp;
                    break;
            }
        } else {
            *kp = *lp;
        }

        lp++;
        kp++;
    }

    *key_size = kp - key;

zacarias_getuserkey_epilogue:

    memset(line, 0, sizeof(line));

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), con_mode);

    if (is_toynix()) {
        stty_echo_on
    }

    return key;
}

#undef stty_echo_on

#undef stty_echo_off

#endif
