/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <kbd/kbd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#if !defined(_WIN32)
# include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <kbd/kmap.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#else
# include <windows.h>
#endif

#if !defined(_WIN32)
static struct termios old, new;
#else
static DWORD con_mode;
#endif

static void (*g_cancel_callout)(void *) = NULL;
static void *g_cancel_callout_args = NULL;
static int g_abort_kbd_rw = 0;

static void getuserkey_sigint_watchdog(int signo);

#if defined(__unix__)
static int zacarias_tty_sendkeys(const kryptos_u8_t *buffer, const size_t buffer_size, const unsigned int timeout_in_secs, void (*cancel_callout)(void *), void *cancel_callout_args);
#endif

int zacarias_sendkeys(const kryptos_u8_t *buffer, const size_t buffer_size, const unsigned int timeout_in_secs, void (*cancel_callout)(void *), void *cancel_callout_args) {
    kryptos_u8_t *input = NULL, *ip = NULL, *ip_end = NULL;
    size_t input_size = 0;
    int err = EXIT_SUCCESS, hold_sh = 0, anulate_sh = 0;
    unsigned int keycode = 0, shiftcode = 0;
    Display *display = NULL;
    kryptos_u8_t abs_key = 0;

    if (buffer == NULL || buffer_size == 0) {
        return EXIT_FAILURE;
    }

    if ((display = XOpenDisplay(NULL)) == NULL) {
        return zacarias_tty_sendkeys(buffer, buffer_size, timeout_in_secs, cancel_callout, cancel_callout_args);
    }

    if (gZacariasCurrKbdLayout->key_demuxer != NULL) {
        input = gZacariasCurrKbdLayout->key_demuxer(buffer, buffer_size, &input_size);
        if (input == NULL) {
            fprintf(stderr, "error: Unable to demux keys from input buffer.\n");
            err = EXIT_FAILURE;
            goto zacarias_sendkeys_epilogue;
        }
    } else {
        input = (kryptos_u8_t *)buffer;
        input_size = buffer_size;
    }

    g_cancel_callout = cancel_callout;
    g_cancel_callout_args = cancel_callout_args;
    g_abort_kbd_rw = 0;

    shiftcode = XKeysymToKeycode(display, (KeySym)XK_Shift_L);

    ip = input;
    ip_end = input + input_size;

    sleep(timeout_in_secs);

    while (ip != ip_end && !g_abort_kbd_rw) {
        if (*ip == 0) {
            anulate_sh = 1;
            ip++;
            continue;
        }

        abs_key = gZacariasCurrKbdLayout->key_mapper(*ip, &hold_sh);

        if (abs_key == 0) {
            fprintf(stderr, "error: Unsupported symbol. Aborted. Did you specified the right keyboard location? (%c %d)\n", *ip, *ip);
            err = 1;
            goto zacarias_sendkeys_epilogue;
        }

        keycode = XKeysymToKeycode(display, (KeySym)abs_key);

        if (anulate_sh) {
            hold_sh = anulate_sh = 0;
        }

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

    if (g_abort_kbd_rw) {
        err = EXIT_FAILURE;
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

    if (name == NULL) {
        return 0;
    }

    while (kp != kp_end) {
        if (strcmp(kp->layout.name, name) == 0) {
            gZacariasCurrKbdLayout = &kp->layout;
            return 1;
        }
        kp++;
    }

    return 0;
}

static int zacarias_tty_sendkeys(const kryptos_u8_t *buffer, const size_t buffer_size, const unsigned int timeout_in_secs, void (*cancel_callout)(void *), void *cancel_callout_args) {
    int dev_console;
    const kryptos_u8_t *bp;
    const kryptos_u8_t *bp_end;

    if (buffer == NULL || buffer_size == 0) {
        return EXIT_FAILURE;
    }

    if ((dev_console = open("/dev/console", O_WRONLY | O_NONBLOCK)) == -1) {
        return EXIT_FAILURE;
    }

    g_cancel_callout = cancel_callout;
    g_cancel_callout_args = cancel_callout_args;
    g_abort_kbd_rw = 0;

    bp = buffer;
    bp_end = bp + buffer_size;

    sleep(timeout_in_secs);

    while (bp != bp_end && !g_abort_kbd_rw) {
        ioctl(dev_console, TIOCSTI, &bp[0]);
        bp++;
    }

    close(dev_console);

    return (!g_abort_kbd_rw) ? EXIT_SUCCESS : EXIT_FAILURE;
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
    if (g_cancel_callout != NULL) {
        g_cancel_callout(g_cancel_callout_args);
        g_cancel_callout = NULL;
        g_cancel_callout_args = NULL;
    }
    g_abort_kbd_rw = 1;
    //exit(1);
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

    g_abort_kbd_rw = 0;

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

    while (lp < lp_end && !g_abort_kbd_rw) {
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

    g_abort_kbd_rw = 0;

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

    while (lp < lp_end && !g_abort_kbd_rw) {
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
