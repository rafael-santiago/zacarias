/*
 *                    Copyright (C) 2020, 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <cmd/utils.h>
#include <cutest.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int zc(const char *command, const char *args, const char *keyboard_data);
static int traced_zc(const char *command, const char *args, const char *keyboard_data);
static int zacarias_install(void);
static int zacarias_uninstall(void);
static int can_run_syscall_tracing_tests(void);

CUTE_DECLARE_TEST_CASE(cmd_tests);
CUTE_DECLARE_TEST_CASE(get_canonical_path_tests);
CUTE_DECLARE_TEST_CASE(device_install_tests);
CUTE_DECLARE_TEST_CASE(device_uninstall_tests);
CUTE_DECLARE_TEST_CASE(attach_tests);
CUTE_DECLARE_TEST_CASE(detach_tests);
CUTE_DECLARE_TEST_CASE(password_add_tests);
CUTE_DECLARE_TEST_CASE(password_del_tests);
CUTE_DECLARE_TEST_CASE(password_get_tests);
CUTE_DECLARE_TEST_CASE(regular_using_tests);
CUTE_DECLARE_TEST_CASE(syscall_tracing_mitigation_tests);

CUTE_MAIN(cmd_tests);

CUTE_TEST_CASE(cmd_tests)
    CUTE_RUN_TEST(get_canonical_path_tests);
    CUTE_RUN_TEST(device_install_tests);
    CUTE_RUN_TEST(device_uninstall_tests);
    CUTE_RUN_TEST(attach_tests);
    CUTE_RUN_TEST(detach_tests);
    CUTE_RUN_TEST(password_add_tests);
    CUTE_RUN_TEST(password_del_tests);
    CUTE_RUN_TEST(password_get_tests);
    if (CUTE_GET_OPTION("quick-tests") == NULL) {
        CUTE_RUN_TEST(regular_using_tests);
    } else {
        fprintf(stdout, "WARN: regular_using_tests skipped.\n");
    }
    CUTE_RUN_TEST(syscall_tracing_mitigation_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(syscall_tracing_mitigation_tests)
    if (can_run_syscall_tracing_tests()) {
        remove("passwd");
        zc("device", "uninstall", NULL);
        CUTE_ASSERT(zc("device",
                       "install --device-driver-path=../../dev/zacarias.ko", NULL) == EXIT_SUCCESS);
        CUTE_ASSERT(zc("attach", "--user=rs --pwdb=passwd --init", "1234mudar\n1234mudar\n") == EXIT_SUCCESS);
        CUTE_ASSERT(zc("password", "add --user=rs --alias=syscall_tr@test.com",
                       "1234mudar\n123\n123\n") == EXIT_SUCCESS);

        CUTE_ASSERT(traced_zc("password",
                              "get --user=rs --alias=syscall_tr@test.com", "1234mudar\n") != EXIT_SUCCESS);

        CUTE_ASSERT(zc("detach", "--user=rs", "1234mudar\n") == EXIT_SUCCESS);

        CUTE_ASSERT(traced_zc("attach", "--user=rs --pwdb=passwd", "1234mudar\n") != EXIT_SUCCESS);

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
    CUTE_ASSERT(zc("device", "install --device-driver-path=../../dev/zacarias.ko", NULL) == EXIT_SUCCESS);
    CUTE_ASSERT(zc("device", "install --device-driver-path=../../dev/zacarias.ko", NULL) != EXIT_SUCCESS);
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

    snprintf(expected, sizeof(expected) - 1, "%s/passwd", cwd);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "passwd", 6) == &result[0]);
    CUTE_ASSERT(memcmp(result, expected, strlen(expected)) == 0);

    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "404/passwd", 10) == NULL);

    snprintf(input, sizeof(input) - 1, "%s/404/passwd", cwd);
    printf("%s\n", result);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, input, strlen(input)) == NULL);

    CUTE_ASSERT(mkdir("404", 0666) == EXIT_SUCCESS);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, input, strlen(input)) == &result[0]);
    CUTE_ASSERT(memcmp(result, input, strlen(input)) == 0);

    CUTE_ASSERT(chdir("404") == EXIT_SUCCESS);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "../404/passwd", 13) == &result[0]);
    CUTE_ASSERT(memcmp(result, input, strlen(input)) == 0);

    snprintf(expected, sizeof(expected) - 1, "%s/passwd", cwd);
    CUTE_ASSERT(get_canonical_path(result, sizeof(result) - 1, "../passwd", 9) == &result[0]);
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

    CUTE_ASSERT(mkdir("tmp", 0666) == EXIT_SUCCESS);
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

static int zc(const char *command, const char *args, const char *keyboard_data) {
#if defined(__unix__)
    const char zc_binary[] = "bin/zc";
#else
# error Some code wanted.
#endif
    char command_line[4096];
    FILE *fp;
    int exit_code = EXIT_FAILURE;
    char *kbd_input = "";
    struct stat st;
    char backbuf[4096] = "";

#if defined(__unix__)
    do {
        if (strlen(backbuf) > 4000) {
            break;
        }
        strcat(backbuf, "../");
        snprintf(command_line, sizeof(command_line) - 1, "%s%s", backbuf, zc_binary);
    } while (stat(command_line, &st) != EXIT_SUCCESS);
#endif

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
        "truss --version >/dev/null 2>&1";
#endif
    return (system(cmdline) == 0);
}