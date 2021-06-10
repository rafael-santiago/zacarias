#include <cutest.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

static int zc(const char *command, const char *args, const char *keyboard_data);
static int zacarias_install(void);
static int zacarias_uninstall(void);


CUTE_DECLARE_TEST_CASE(cmd_tests);
CUTE_DECLARE_TEST_CASE(attach_tests);
CUTE_DECLARE_TEST_CASE(detach_tests);

CUTE_MAIN(cmd_tests);

CUTE_TEST_CASE(cmd_tests)
    CUTE_RUN_TEST(attach_tests);
    CUTE_RUN_TEST(detach_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(attach_tests)
    char cwd[4096];
    char args[4096];
    struct stat st;

    remove("passwd");
    zacarias_uninstall();
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    sleep(3);

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

    sleep(3);


    remove("passwd");

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n112\n113\n") != EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) != EXIT_SUCCESS);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned --init", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n123mudar*\n112\n112\n") == EXIT_SUCCESS);
    CUTE_ASSERT(stat("passwd", &st) == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    sleep(3);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n") == EXIT_SUCCESS);

    CUTE_ASSERT(zacarias_uninstall() == EXIT_SUCCESS);
    CUTE_ASSERT(zacarias_install() == EXIT_SUCCESS);

    sleep(3);

    snprintf(args, sizeof(args) - 1, "--pwdb=%s/passwd --user=rs --sessioned", cwd);
    CUTE_ASSERT(zc("attach", args, "123mudar*\n1#12\n1#12\n") == EXIT_SUCCESS);

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
    const char zc_binary[] = "../../../bin/zc";
#else
# error Some code wanted.
#endif
    char command_line[4096];
    FILE *fp;
    int exit_code = EXIT_FAILURE;
    char *kbd_input = "";

    if (keyboard_data != NULL) {
        fp = fopen(".keybd_data", "wb");
        if (fp == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(fp, "%s", keyboard_data);
        fclose(fp);
        kbd_input = "< .keybd_data";
    }

    snprintf(command_line, sizeof(command_line) - 1, "%s %s %s %s", zc_binary, command, (args != NULL) ? args : "", kbd_input);

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

#if defined(__linux__)
    snprintf(command_line, sizeof(command_line) - 1, "insmod %s", zacarias_lkm);
#else
    return EXIT_FAILURE;
#endif

    return system(command_line);
}

static int zacarias_uninstall(void) {
#if defined(__unix__)
    const char zacarias_lkm[] = "zacarias.ko";
#else
# error Some code wanted.
#endif
    char command_line[4096];

#if defined(__linux__)
    snprintf(command_line, sizeof(command_line) - 1, "rmmod %s", zacarias_lkm);
#else
    return EXIT_FAILURE;
#endif

    return system(command_line);
}
