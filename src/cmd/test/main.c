/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/utils.h>
#include <cmd/strglob.h>
#include <cutest.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
# include <windows.h>
#endif

static int zc(const char *command, const char *args, const char *keyboard_data);
static int traced_zc(const char *command, const char *args, const char *keyboard_data);
static int zacarias_install(void);
static int zacarias_uninstall(void);
static int can_run_syscall_tracing_tests(void);
static int has_gdb(void);
static int has_lldb(void);
static FILE *gdb(const char *stderr_path);
static void gdb_run(FILE *gdb_proc, const char *command_line);
static void gdb_quit(FILE *gdb_proc);
static void gdb_target_exec(FILE *gdb_proc, const char *binary_path);
static FILE *lldb(const char *stderr_path);
static void lldb_run(FILE *lldb_proc, const char *command_line);
static void lldb_quit(FILE *lldb_proc);
static void lldb_file(FILE *lldb_proc, const char *binary_path);

CUTE_DECLARE_TEST_CASE(cmd_tests);
CUTE_DECLARE_TEST_CASE(get_canonical_path_tests);
CUTE_DECLARE_TEST_CASE(device_install_tests);
CUTE_DECLARE_TEST_CASE(device_uninstall_tests);
CUTE_DECLARE_TEST_CASE(attach_tests);
CUTE_DECLARE_TEST_CASE(detach_tests);
CUTE_DECLARE_TEST_CASE(password_add_tests);
CUTE_DECLARE_TEST_CASE(password_del_tests);
CUTE_DECLARE_TEST_CASE(password_get_tests);
CUTE_DECLARE_TEST_CASE(password_aliases_tests);
CUTE_DECLARE_TEST_CASE(device_install_uninstall_stressing_tests);
CUTE_DECLARE_TEST_CASE(regular_using_tests);
CUTE_DECLARE_TEST_CASE(syscall_tracing_mitigation_tests);
CUTE_DECLARE_TEST_CASE(debugging_avoidance_tests);
CUTE_DECLARE_TEST_CASE(strglob_tests);

#if defined(_WIN32)
CUTE_DECLARE_TEST_CASE(get_ntpath_tests);
#endif

CUTE_MAIN(cmd_tests);

CUTE_TEST_CASE(cmd_tests)
    CUTE_RUN_TEST(get_canonical_path_tests);
#if defined(_WIN32)
    CUTE_RUN_TEST(get_ntpath_tests);
#endif
    CUTE_RUN_TEST(strglob_tests);
    CUTE_RUN_TEST(device_install_tests);
    CUTE_RUN_TEST(device_uninstall_tests);
    CUTE_RUN_TEST(attach_tests);
    CUTE_RUN_TEST(detach_tests);
    CUTE_RUN_TEST(password_add_tests);
    CUTE_RUN_TEST(password_del_tests);
    CUTE_RUN_TEST(password_aliases_tests);
    CUTE_RUN_TEST(password_get_tests);
    if (CUTE_GET_OPTION("quick-tests") == NULL) {
        CUTE_RUN_TEST(device_install_uninstall_stressing_tests);
        CUTE_RUN_TEST(regular_using_tests);
    } else {
        fprintf(stdout, "WARN: device_install_uninstall_stressing_tests skipped.\n");
        fprintf(stdout, "WARN: regular_using_tests skipped.\n");
    }
#if !defined(_WIN32)
    CUTE_RUN_TEST(syscall_tracing_mitigation_tests);
#endif
#if !defined(__linux__)
    if (CUTE_GET_OPTION("quick-tests") == NULL) {
        CUTE_RUN_TEST(debugging_avoidance_tests);
    }
#endif
CUTE_TEST_CASE_END

CUTE_TEST_CASE(debugging_avoidance_tests)
#if defined(_WIN32)
    char *install_cmd = "install --device-driver-path=..\\..\\dev\\zacarias.sys";
    char *zc_path = "..\\..\\..\\bin\\zc.exe";
#elif defined(__linux__) || defined(__FreeBSD__)
    char *install_cmd = "install --device-driver-path=../../dev/zacarias.ko";
    char *zc_path = "../../../bin/zc";
#else
# error Some code wanted.
#endif
    pid_t pid = 0;
    FILE *debugger = NULL, *fp = NULL;
    char buf[65535];
    struct stat st;

    if (has_gdb()) {
        printf("*** Testing anti-debugging against GDB...\n");
        zc("device", "uninstall", NULL);
        CUTE_ASSERT(zc("device", install_cmd, NULL) == EXIT_SUCCESS);
        remove("gdb-stderr.txt");
        debugger = gdb("gdb-stderr.txt");
        CUTE_ASSERT(debugger != NULL);
        gdb_target_exec(debugger, zc_path);
        gdb_run(debugger, "attach --user=rs --pwdb=test.db --init");
        sleep(5);
        fprintf(debugger, "abc\nabc\n");
        gdb_quit(debugger);
        pclose(debugger);
        CUTE_ASSERT(zc("device", "uninstall", NULL) == EXIT_SUCCESS);
#if defined(_WIN32)
        Sleep(5000);
#else
        sleep(5);
#endif
        CUTE_ASSERT(stat("gdb-stderr.txt", &st) == EXIT_SUCCESS);
        fp = fopen("gdb-stderr.txt", "r");
        CUTE_ASSERT(fp != NULL);
        fread(buf, 1, sizeof(buf), fp);
        fclose(fp);
        CUTE_ASSERT(strstr(buf, "ALERT: A debugger attachment was detect. Aborting execution to avoid more damage.") != NULL);
        CUTE_ASSERT(strstr(buf, "Do not execute zc again until make sure that your system is clean.") != NULL);
        remove("gdb-stderr.txt");
    }

    if (has_lldb()) {
        printf("*** Testing anti-debugging against LLDB...\n");
        zc("device", "uninstall", NULL);
        CUTE_ASSERT(zc("device", install_cmd, NULL) == EXIT_SUCCESS);
        remove("lldb-stderr.txt");
        debugger = lldb("lldb-stderr.txt");
        CUTE_ASSERT(debugger != NULL);
        lldb_file(debugger, zc_path);
        lldb_run(debugger, "attach --user=rs --pwdb=test.db --init");
        sleep(5);
        fprintf(debugger, "abc\nabc\n");
        fflush(debugger);
        lldb_quit(debugger);
        pclose(debugger);
        CUTE_ASSERT(zc("device", "uninstall", NULL) == EXIT_SUCCESS);
#if defined(_WIN32)
        Sleep(5000);
#else
        sleep(5);
#endif
        CUTE_ASSERT(stat("lldb-stderr.txt", &st) == EXIT_SUCCESS);
        fp = fopen("lldb-stderr.txt", "r");
        CUTE_ASSERT(fp != NULL);
        fread(buf, 1, sizeof(buf), fp);
        fclose(fp);
        CUTE_ASSERT(strstr(buf, "ALERT: A debugger attachment was detect. Aborting execution to avoid more damage.") != NULL);
        CUTE_ASSERT(strstr(buf,"Do not execute zc again until make sure that your system is clean.") != NULL);
        remove("lldb-stderr.txt");
    }
CUTE_TEST_CASE_END

#if defined(_WIN32)
CUTE_TEST_CASE(get_ntpath_tests)
    const char *path = "C:\\Whatever\\abc.txt";
    char dest[MAX_PATH];
    size_t dest_size;
    char expected[MAX_PATH];
    CUTE_ASSERT(QueryDosDeviceA("C:", expected, sizeof(expected) - 1) != 0);
    strcat(expected, "\\Whatever\\abc.txt");
    CUTE_ASSERT(get_ntpath(NULL, sizeof(dest) - 1, path, strlen(path)) == NULL);
    CUTE_ASSERT(get_ntpath(dest, 0, path, strlen(path)) == NULL);
    CUTE_ASSERT(get_ntpath(dest, sizeof(dest) - 1, NULL, strlen(path)) == NULL);
    CUTE_ASSERT(get_ntpath(dest, sizeof(dest) - 1, path, 0) == NULL);
    CUTE_ASSERT(get_ntpath(dest, sizeof(dest) - 1, &path[2], strlen(&path[2])) == NULL);
    CUTE_ASSERT(get_ntpath(dest, sizeof(dest) - 1, path, strlen(path)) == &dest[0]);
    dest_size = strlen(dest);
    CUTE_ASSERT(dest_size == strlen(expected));
    CUTE_ASSERT(memcmp(dest, expected, dest_size) == 0);
CUTE_TEST_CASE_END
#endif

CUTE_TEST_CASE(device_install_uninstall_stressing_tests)
    // INFO(Rafael): An important regression testing for Linux device.
    //               Since char device creation on Linux is everything except simple,
    //               there was a bug on it that was causing failures when attempting
    //               many insmod/rmmods. Now it is being done correctly according to
    //               the new but (still complicated) way. Anyway, executing it on
    //               FreeBSD and Windows will no hurt.
    const size_t attempts_nr = 10000;
    size_t a;
    for (a = 0; a < attempts_nr; a++) {
        printf("%.0f%% completed...\r", ((float)a / (float)attempts_nr) * 100);
#if defined(__linux__) || defined(__FreeBSD__)
        CUTE_ASSERT(zc("device", "install --device-driver-path=../../dev/zacarias.ko", NULL) == EXIT_SUCCESS);
#elif defined(_WIN32)
        CUTE_ASSERT(zc("device", "install --device-driver-path=..\\..\\dev\\zacarias.sys", NULL) == EXIT_SUCCESS);
#endif
        CUTE_ASSERT(zc("device", "uninstall", NULL) == EXIT_SUCCESS);
    }
    printf("                            \r");
CUTE_TEST_CASE_END

CUTE_TEST_CASE(syscall_tracing_mitigation_tests)
    FILE *fp = NULL;
    char buf[4096];
    if (can_run_syscall_tracing_tests()) {
        remove("passwd");
        zc("device", "uninstall", NULL);
        CUTE_ASSERT(zc("device",
                       "install --device-driver-path=../../dev/zacarias.ko", NULL) == EXIT_SUCCESS);
        CUTE_ASSERT(zc("attach", "--user=rs --pwdb=passwd --init", "1234mudar\n1234mudar\n") == EXIT_SUCCESS);
        CUTE_ASSERT(zc("password", "add --user=rs --alias=syscall_tr@test.com",
                       "1234mudar\n123\n123\n") == EXIT_SUCCESS);

#if defined(__linux__)
        CUTE_ASSERT(traced_zc("password",
                              "get --user=rs --alias=syscall_tr@test.com", "1234mudar\n") != EXIT_SUCCESS);
#elif defined(__FreeBSD__)
        CUTE_ASSERT(traced_zc("password",
                              "get --user=rs --alias=syscall_tr@test.com", "1234mudar\n") == EXIT_SUCCESS);
        fp = fopen("out.txt", "rb");
        CUTE_ASSERT(fp != NULL);
        memset(buf, 0, sizeof(buf));
        fread(buf, 1, sizeof(buf) - 1, fp);
        fclose(fp);
        CUTE_ASSERT(strstr(buf, "1234mudar") == NULL);
#endif

        CUTE_ASSERT(zc("detach", "--user=rs", "1234mudar\n") == EXIT_SUCCESS);

#if defined(__linux__)
        CUTE_ASSERT(traced_zc("attach", "--user=rs --pwdb=passwd", "1234mudar\n") != EXIT_SUCCESS);
#elif defined(__FreeBSD__)
        CUTE_ASSERT(traced_zc("password",
                              "get --user=rs --alias=syscall_tr@test.com", "1234mudar\n") == EXIT_SUCCESS);
        fp = fopen("out.txt", "rb");
        CUTE_ASSERT(fp != NULL);
        memset(buf, 0, sizeof(buf));
        fread(buf, 1, sizeof(buf) - 1, fp);
        fclose(fp);
        CUTE_ASSERT(strstr(buf, "1234mudar") == NULL);
#endif

        CUTE_ASSERT(zc("device", "uninstall", NULL) == EXIT_SUCCESS);
        remove("passwd");
        remove("out.txt");
    } else {
        fprintf(stdout, "WARN: your system does not have any syscall "
                        "tracing capabilities. This tests was skipped.\n");
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(device_install_tests)
    zc("device", "uninstall", NULL);
    CUTE_ASSERT(zc("device", "install", NULL) != EXIT_SUCCESS);
#if defined(__linux__) || defined(__FreeBSD__)
    CUTE_ASSERT(zc("device", "install --device-driver-path=../../dev/zacarias.ko", NULL) == EXIT_SUCCESS);
    CUTE_ASSERT(zc("device", "install --device-driver-path=../../dev/zacarias.ko", NULL) != EXIT_SUCCESS);
#elif defined(_WIN32)
    CUTE_ASSERT(zc("device", "install --device-driver-path=..\\..\\dev\\zacarias.sys", NULL) == EXIT_SUCCESS);
    CUTE_ASSERT(zc("device", "install --device-driver-path=..\\..\\dev\\zacarias.sys", NULL) != EXIT_SUCCESS);
#else
# error Some code wanted.
#endif
CUTE_TEST_CASE_END

CUTE_TEST_CASE(device_uninstall_tests)
    CUTE_ASSERT(zc("device", "uninstall", NULL) == EXIT_SUCCESS);
    CUTE_ASSERT(zc("device", "uninstall", NULL) != EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(get_canonical_path_tests)
    char cwd[4096];
    char expected[4096];
    char result[4096];
    char input[4096];

    rmdir("404");
    CUTE_ASSERT(getcwd(cwd, sizeof(cwd) - 1) != NULL);

    CUTE_ASSERT(get_canonical_path(NULL, sizeof(result) - 1, "passwd", 6) == NULL);
    CUTE_ASSERT(get_canonical_path(result, 0, "passwd", 6) == NULL);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, NULL, 6) == NULL);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "passwd", 0) == NULL);

#if defined(__unix__)
    snprintf(expected, sizeof(expected) - 1, "%s/passwd", cwd);
#else
    snprintf(expected, sizeof(expected) - 1, "%s\\passwd", cwd);
#endif
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "passwd", 6) == &result[0]);
    CUTE_ASSERT(memcmp(result, expected, strlen(expected)) == 0);

#if defined(__unix__)
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "404/passwd", 10) == NULL);
#endif

#if defined(__unix__)
    snprintf(input, sizeof(input) - 1, "%s/404/passwd", cwd);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, input, strlen(input)) == NULL);
#elif defined(_WIN32)
    snprintf(input, sizeof(input) - 1, "%s\\404\\passwd", cwd);
#endif

#if defined(__unix__)
    CUTE_ASSERT(mkdir("404", 0666) == EXIT_SUCCESS);
#elif defined(_WIN32)
    CUTE_ASSERT(mkdir("404") == EXIT_SUCCESS);
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, input, strlen(input)) == &result[0]);
    CUTE_ASSERT(memcmp(result, input, strlen(input)) == 0);

    CUTE_ASSERT(chdir("404") == EXIT_SUCCESS);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "../404/passwd", 13) == &result[0]);
    CUTE_ASSERT(memcmp(result, input, strlen(input)) == 0);

#if defined(__unix__)
    snprintf(expected, sizeof(expected) - 1, "%s/passwd", cwd);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "../passwd", 9) == &result[0]);
#elif defined(_WIN32)
    snprintf(expected, sizeof(expected) - 1, "%s\\passwd", cwd);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "..\\passwd", 9) == &result[0]);
#endif
    CUTE_ASSERT(memcmp(result, expected, strlen(expected)) == 0);

    CUTE_ASSERT(chdir("..") == EXIT_SUCCESS);
    rmdir("404");
CUTE_TEST_CASE_END

CUTE_TEST_CASE(regular_using_tests)
    struct stat st;

    remove("passwd");
    zacarias_uninstall();

    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init",
                   "GiveTheMUleWhatHeWants\nGiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(stat("passwd", &st) != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init",
                   "GiveTheMuleWhatHeWants\nGiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\nCryCryButterfly\nCryCryButFly\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\nCryCryButterfly\nCryCryButterfly\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\nCryCryButterfly\nCryCryButterfly\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=dirt_in_the_ground@tom.waits",
                   "GiveTheMuleWhatHeWants\nDrivingBonesToLive\nDrivingBonesToLive\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=butterfly@screaming.trees "
                               "--timeout=1", "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rafael --alias=butterfly@screaming.trees",
                   "GiveTheMuleWatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rafael --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    remove("useless-things-i-forgot");
    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=useless-things-i-forgot --init",
                   "MonkeyBoy\nMonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(stat("useless-things-i-forgot", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --alias=grendel-snowman@fu.manchu --user=zaca",
                   "MonkeyBoiMoooo\nThereHeGoes\nThereHeGoes\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --alias=grendel-snowman@fu.manchu --user=zaca",
                   "MonkeyBoy\nThereHeGoes\nThereHeGoes\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --alias=the_boxer@simon.garfunkel --user=zaca",
                   "MonkeyBoy\nLieLaLie\nLieLaLie\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=butterfly@screaming.trees",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=dirt_in_the_ground@tom.waits",
                   "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=the_boxer@simon.garfunkel "
                               "--timeout=1", "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "GiveTheMuleWhatHeWants.\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=grendel-snowman@fu.manchu",
                   "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=dirt_in_the_ground@tom.waits",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=funky-monks --sessioned",
                   "GiveTheMuleWhatHeWants\nJabulaniDaSilva\nJabulaniDaSilva\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --sessioned",
                   "GiveTheMuleWhatHeWants\nFunkyMonks\nFunkyMonks\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=dirt_in_the_ground@tom.waits",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=dirt_in_the_ground@tom.waits "
                               "--timeout=1", "FunkyMonks\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=the_boxer@simon.garfunkel",
                   "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=the_boxer@simon.garfunkel",
                   "FunkyMonks\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=the_boxer@simon.garfunkel --timeout=1",
                   "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=funland_at_the_beach@dead.kennedys --sessioned",
                   "GiveTheMuleWhatHeWants\nFunkyMonks\nCrushedLittleKidsAdornTheBoardwalk\n"
                   "CrushedLittleKidsAdornTheBoardwalk\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=funland_at_the_beach@dead.kennedys --timeout=1",
                   "FunkyMonks\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=zaca", "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=useless-things-i-forgot --sessioned",
                   "MonkeyBoy\nGodIsInTheRadioLeakingThroughTheStereo\n"
                   "GodIsInTheRadioLeakingThroughTheStereo\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=zaca --alias=harlen-shuffle@rolling.stones --sessioned",
                   "MonkeyBoy\nGodIsInTheRadioLeakingThroughTheStereo\n"
                   "YeahYeahYeah\nYeahYeahYeah\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=zaca --alias=grendel-snowman@fu.manchu",
                   "MonkeyBoy\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=zaca --alias=grendel-snowman@fu.manchu --sessioned",
                   "MonkeyBoy\nGodIsInTheRadioLeakingThroughTheStereo\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --alias=grendel-snowman@fu.manchu",
                   "GodIsInTheRadioLeakingThroughTheStereo\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --alias=the_boxer@simon.garfunkel --timeout=1 "
                   "--user=zaca", "GodIsInTheRadioLeakingThroughTheStereo\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "GiveTheMuleWhatHeWants\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "FunkyMonks\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=zaca", "MonkeyBoy\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=zaca", "GodIsInTheRadioLeakingThroughTheStereo\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=passwd", "MonkeyBoy\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=passwd",
                   "GodIsInTheRadioLeakingThroughTheStereo\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=useless-things-i-forgot",
                   "GodIsInTheRadioLeakingThroughTheStereo\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=zaca --pwdb=useless-things-i-forgot",
                   "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--user=rs --pwdb=passwd",
                   "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=funland_at_the_beach@dead.kennedys --timeout=1",
                   "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=dirt_in_the_ground@tom.waits --timeout=1",
                   "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=the_boxer@simon.garfunkel",
                   "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=zaca --alias=harlen-shuffle@rolling.stones",
                   "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "aliases --user=zaca", "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "GiveTheMuleWhatHeWants\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=zaca", "MonkeyBoy\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);

    CUTE_ASSERT(remove("passwd") == EXIT_SUCCESS);

    CUTE_ASSERT(remove("useless-things-i-forgot") == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(password_get_tests)
    char args[4096];

    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init",
                   "123mudar*\n123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=zacarias.get_test",
                   "123mudar*\nwabbalabba\nwabbalabba\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=zacarias.get_test",
                   "123macular*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=not.found.aieee", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=zacarias.get_test", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --sessioned", "123mudar*\n***\n***\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=zacarias.get_test", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "get --user=rs --alias=zacarias.get_test", "***\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "***\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    remove("passwd");
CUTE_TEST_CASE_END

CUTE_TEST_CASE(password_del_tests)
    char args[4096];
    struct stat st;

    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init", "123mudar*\n123mudar*\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs1 --alias=404.com", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=404.com", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=404.com", "123mudar*\n123change*\n123change*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=200.com", "123mudar*\nabcd\nabcd\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=404.com", "123change*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=404.com", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=404.com", "123mudar*\n123change*\n123change*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=200.com", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=404.com", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=404.com", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=200.com", "123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --sessioned",
                   "123mudar*\ngoo goo muck\ngoo goo muck\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=200.com --sessioned",
                   "123mudar*\ngoo goo muck\nabcd\nabcd\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=200.com --sessioned", "123mudar*\n123mudar*\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "del --user=rs --alias=200.com --sessioned", "123mudar*\ngoo goo muck\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    remove("passwd");
CUTE_TEST_CASE_END

CUTE_TEST_CASE(password_aliases_tests)
    struct stat st;

    remove("aliases");
    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init", "123mudar@\n123mudar@\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "aliases", "123mudar@\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "aliases --user=meeseeks", "123\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "aliases --user=rs", "123mudar@\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=rick.sanchez@plumb.us",
                   "123mudar@\nburrp!\nburrp!\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=jerry@nowhere",
                   "123mudar@\n1234\n4321\n") != EXIT_SUCCESS);

    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=jerry@nowhere",
                   "123mudar@\n1234\n1234\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=morty@teenagerhood",
                   "123mudar@\njess!ca\njess!ca\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=summer@cellphone",
                   "123mudar@\nethan\nethan\n") == EXIT_SUCCESS);


    CUTE_ASSERT(zc("password",
                   "add --user=rs --alias=beth@wine",
                   "123mudar@\nw!n3\nw!n3\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "aliases --user=rs", "123m?\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "aliases --user=rs aliases.txt", "123mudar@\n") == EXIT_SUCCESS);

    zacarias_uninstall();
    remove("passwd");
CUTE_TEST_CASE_END

CUTE_TEST_CASE(password_add_tests)
    char args[4096];
    struct stat st;

    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init", "123mudar*\n123mudar*\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com", "123mudar\nabc\nabc\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add", "123mudar*\n123mudar\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs", "123mudar*\n123mudar\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com", "123mudar*\nabc\nabd\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com", "123mudar*\nabc\nabc\n") == EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com", "123mudar*\nabc\nabc\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=zeta.com", "123mudar*\nwww\nwww\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") == EXIT_SUCCESS);
    CUTE_ASSERT(remove("passwd") == EXIT_SUCCESS);


    CUTE_ASSERT(zc("attach", "--pwdb=passwd --user=rs --init --sessioned",
                   "123mudar*\n123mudar*\nziriguidum\nziriguidum\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com --sessioned",
                   "123mudar*\n123mudar*\nabc\nabc\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com --sessioned",
                   "123mudar*\nziriguidum\nabc\nabc\n") == EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=alpha.com --sessioned",
                   "123mudar*\nziriguidum\nabc\nabc\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("password", "add --user=rs --alias=zeta.com --sessioned",
                   "123mudar*\nziriguidum\nabc\nabc\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rs", "ziriguidum\n") == EXIT_SUCCESS);
    CUTE_ASSERT(remove("passwd") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(attach_tests)
    char cwd[4096];
    char args[4096];
    struct stat st;

    rmdir("tmp");
    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(zc("attach", NULL, NULL) != EXIT_SUCCESS);

    CUTE_ASSERT(getcwd(cwd, sizeof(cwd) - 1) != NULL);
    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n4321wabba!\n4321wabba!\n") != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar\n") != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "n\n") != EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "y\n123abc\n123\n") != EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "y\n123abc\n123abc\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    remove("passwd");

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n112\n113\n") != EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n112\n112\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n1#12\n1#12\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

#if defined(__unix__)
    CUTE_ASSERT(mkdir("tmp", 0666) == EXIT_SUCCESS);
#elif defined(_WIN32)
    CUTE_ASSERT(mkdir("tmp") == EXIT_SUCCESS);
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(chdir("tmp") == EXIT_SUCCESS);
    snprintf(args, sizeof(args) - 1, "--pwdb=../passwd --user=rs");
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("../passwd", &st) == EXIT_SUCCESS);
    CUTE_ASSERT(chdir("..") == EXIT_SUCCESS);

    rmdir("tmp");
    remove("passwd");
    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(detach_tests)
    char cwd[4096];
    char args[4096];
    struct stat st;

    remove("passwd");

    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    CUTE_ASSERT(getcwd(cwd, sizeof(cwd) - 1) != NULL);
    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "1234mudar*\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rafael-santiago", "123mudar*\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") == EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\nabracadabra\nabracadabra\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zc("detach", "--user=rafael.santiago", "123mudar*\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rs", "123mudar*\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rafael.netto", "abracadabra\n") != EXIT_SUCCESS);
    CUTE_ASSERT(zc("detach", "--user=rs", "abracadabra\n") == EXIT_SUCCESS);

    remove("passwd");
    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(strglob_tests)
    struct strglob_tests_ctx {
        const char *str;
        const char *pattern;
        int result;
    };
    struct strglob_tests_ctx tests[] = {
        { NULL,                         NULL                                                       , 0 },
        { "abc",                        "abc"                                                      , 1 },
        { "abc",                        "ab"                                                       , 0 },
        { "abc",                        "a?c"                                                      , 1 },
        { "abc",                        "ab[abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.c]", 1 },
        { "abc",                        "ab[abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.?]", 0 },
        { "ab*",                        "ab[c*]"                                                   , 1 },
        { "ab*",                        "ab[*c]"                                                   , 1 },
        { "abc",                        "ab*"                                                      , 1 },
        { "abc",                        "abc*"                                                     , 1 },
        { "strglob.c",                  "strglo*.c"                                                , 1 },
        { "parangaricutirimirruaru!!!", "*"                                                        , 1 },
        { "parangaritititero",          "?"                                                        , 0 },
        { "parangaritititero",          "?*"                                                       , 1 },
        { "parangaricutirimirruaru",    "paran*"                                                   , 1 },
        { "parangaricutirimirruaru",    "parruari"                                                 , 0 },
        { "parangaricutirimirruaru",    "paran*garicuti"                                           , 0 },
        { "parangaricutirimirruaru",    "paran*garicutirimirruaru"                                 , 1 },
        { "parangaricutirimirruaru",    "paran*ru"                                                 , 1 },
        { "hell yeah!",                 "*yeah!"                                                   , 1 },
        { ".",                          "*[Gg]lenda*"                                              , 0 },
    };
    size_t tests_nr = sizeof(tests) / sizeof(tests[0]), t;

    for (t = 0; t < tests_nr; t++) {
        CUTE_ASSERT(strglob(tests[t].str, tests[t].pattern) == tests[t].result);
    }
CUTE_TEST_CASE_END

static int zc(const char *command, const char *args, const char *keyboard_data) {
#if defined(__unix__)
    const char zc_binary[] = "bin/zc";
#elif defined(_WIN32)
    const char zc_binary[] = "bin\\zc.exe";
#else
# error Some code wanted.
#endif
    char command_line[4096];
    FILE *fp;
    int exit_code = EXIT_FAILURE;
    char *kbd_input = "";
    struct stat st;
    char backbuf[4096] = "";

    do {
        if (strlen(backbuf) > 4000) {
            break;
        }
#if defined(__unix__)
        strcat(backbuf, "../");
#elif defined(_WIN32)
        strcat(backbuf, "..\\");
#else
# error Some code wanted.
#endif
        snprintf(command_line, sizeof(command_line) - 1, "%s%s", backbuf, zc_binary);
    } while (stat(command_line, &st) != EXIT_SUCCESS);

    if (keyboard_data != NULL) {
        fp = fopen(".keybd_data", "wb");
        if (fp == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(fp, "%s", keyboard_data);
        fclose(fp);
        kbd_input = "< .keybd_data";
    }

    snprintf(command_line, sizeof(command_line) - 1, "%s%s %s %s %s",
             backbuf, zc_binary, command, (args != NULL) ? args : "", kbd_input);

    exit_code = system(command_line);

    remove(".keybd_data");

    return exit_code;
}

static int traced_zc(const char *command, const char *args, const char *keyboard_data) {
    const char zc_binary[] = "bin/zc";
    const char systr_cmd[] =
#if defined(__linux__)
        "strace -o out.txt";
#elif defined(__FreeBSD__)
        "truss -o out.txt";
#else
        "";
#endif
    char command_line[4096];
    FILE *fp;
    int exit_code = EXIT_FAILURE;
    char *kbd_input = "";
    struct stat st;
    char backbuf[4096] = "";

    do {
        if (strlen(backbuf) > 4000) {
            break;
        }
        strcat(backbuf, "../");
        snprintf(command_line, sizeof(command_line) - 1, "%s%s", backbuf, zc_binary);
    } while (stat(command_line, &st) != EXIT_SUCCESS);

    if (keyboard_data != NULL) {
        fp = fopen(".keybd_data", "wb");
        if (fp == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(fp, "%s", keyboard_data);
        fclose(fp);
        kbd_input = "< .keybd_data";
    }

    snprintf(command_line, sizeof(command_line) - 1, "%s %s%s %s %s %s",
             systr_cmd, backbuf, zc_binary, command, (args != NULL) ? args : "", kbd_input);

    exit_code = system(command_line);

    remove(".keybd_data");

    return exit_code;
}

static int zacarias_install(void) {
#if defined(__unix__)
    const char zacarias_lkm[] = "../../dev/zacarias.ko";
#elif defined(_WIN32)
    const char zacarias_lkm[] = "..\\..\\dev\\zacarias.sys";
#else
# error Some code wanted.
#endif
    char command_line[4096];

    snprintf(command_line, sizeof(command_line) - 1, "install --device-driver-path=%s", zacarias_lkm);
    return zc("device", command_line, NULL);
}

static int zacarias_uninstall(void) {
    return zc("device", "uninstall", NULL);
}

static int can_run_syscall_tracing_tests(void) {
    const char *cmdline =
#if defined(__linux__)
        "strace -V >/dev/null 2>&1";
#elif defined(__FreeBSD__)
        "truss truss >/dev/null 2>&1";
#else
        "";
#endif
    return (system(cmdline) == 0);
}

static int has_gdb(void) {
#if defined(__unix__)
    return (system("gdb --version >/dev/null 2>&1") == 0);
#elif defined(_WIN32)
    return (system("gdb --version >nul 2>&1") == 0);
#else
# error Some code wanted.
#endif
}

static int has_lldb(void) {
#if defined(__unix__)
    return (system("lldb --version >/dev/null 2>&1") == 0);
#elif defined(_WIN32)
    return (system("lldb --version >nul 2>&1") == 0);
#else
# error Some code wanted.
#endif
}

static FILE *gdb(const char *stderr_path) {
    char cmdline[4096];
    snprintf(cmdline, sizeof(cmdline) - 1, "gdb >%s 2>&1", stderr_path);
    return popen(cmdline, "w");
}

static void gdb_target_exec(FILE *gdb_proc, const char *binary_path) {
    char cmd[4096];
    snprintf(cmd, sizeof(cmd) - 1, "target exec %s\n", binary_path);
    fprintf(gdb_proc, "%s", cmd);
}

static void gdb_run(FILE *gdb_proc, const char *command_line) {
    fprintf(gdb_proc, "run %s\n", command_line);
}

static void gdb_quit(FILE *gdb_proc) {
    fprintf(gdb_proc, "quit\n");
    fflush(gdb_proc);
}

static FILE *lldb(const char *stderr_path) {
    char cmdline[4096];
    snprintf(cmdline, sizeof(cmdline) - 1, "lldb >%s 2>&1", stderr_path);
    return popen(cmdline, "w");
}

static void lldb_file(FILE *lldb_proc, const char *binary_path) {
    char cmdline[4096];
    snprintf(cmdline, sizeof(cmdline) - 1, "file %s\n", binary_path);
    fprintf(lldb_proc, "%s", cmdline);
}

static void lldb_run(FILE *lldb_proc, const char *command_line) {
    fprintf(lldb_proc, "run %s\n", command_line);
}

static void lldb_quit(FILE *lldb_proc) {
    fprintf(lldb_proc, "quit\n");
}
