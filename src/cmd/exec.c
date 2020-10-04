#include <cmd/exec.h>
#include <cmd/types.h>
#include <cmd/options.h>
#include <string.h>
#include <stdio.h>

static int zc_unk_command(void);

int zc_exec(const int argc, char **argv) {
    zc_cmd_func zc_cmd = zc_unk_command;
    struct zc_exec_table_ctx *zetc, *zetc_end;
    char *cmd_name = "";

    zc_set_argc_argv(argc, argv);

    if ((cmd_name = zc_get_command()) == NULL) {
        fprintf(stderr, "ERROR: No command was passed.\n");
        return 1;
    }

    if (strcmp(cmd_name, "--version") == 0) {
        cmd_name = "version";
    }

    zetc = &g_command_table[0];
    zetc_end = zetc + g_command_table_nr;

    while (zetc != zetc_end && zc_cmd == zc_unk_command) {
        if (strcmp(zetc->cmd_name, cmd_name) == 0) {
            zc_cmd = zetc->cmd_do;
        }
        zetc++;
    }

    return zc_cmd();
}

static int zc_unk_command(void) {
    fprintf(stderr, "ERROR: unknown command.\n");
    return 1;
}
