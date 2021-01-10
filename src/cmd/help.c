#include <cmd/help.h>
#include <cmd/types.h>
#include <cmd/options.h>
#include <cmd/didumean.h>
#include <string.h>
#include <stdio.h>

extern struct zc_exec_table_ctx g_command_table[];
extern size_t g_command_table_nr;

int zc_help(void) {
    char *topic = zc_get_subcommand();
    zc_cmd_func do_help = zc_help_help;
    struct zc_exec_table_ctx *cmd = NULL, *cmd_end = NULL;
    const char **avail_commands = NULL;
    char **suggestions = NULL;
    size_t avail_commands_nr = 0;
    char **psug = NULL, **psug_end = NULL;

    if (topic != NULL) {
        cmd_end = &g_command_table[0] + g_command_table_nr;
        for (cmd = &g_command_table[0]; cmd != cmd_end; cmd++) {
            if (strcmp(cmd->cmd_name, topic) == 0) {
                do_help = cmd->cmd_help;
                break;
            }
        }
        if (do_help == zc_help_help) {
            fprintf(stderr, "ERROR: '%s' is an unknown help topic.", topic);
            avail_commands = (const char **) malloc(sizeof(char **) * g_command_table_nr);
            suggestions = (char **) malloc(sizeof(char **) * g_command_table_nr);
            if (avail_commands != NULL && suggestions != NULL) {
                cmd = &g_command_table[0];
                cmd_end = cmd + g_command_table_nr;
                while (cmd != cmd_end) {
                    avail_commands[avail_commands_nr++] = (char *)&cmd->cmd_name[0];
                    cmd++;
                }
                didumean(topic, suggestions, avail_commands_nr, avail_commands, avail_commands_nr, 2);
                if (suggestions[0] != NULL) {
                    fprintf(stderr, "\n\nDid you mean ");
                    psug = suggestions;
                    psug_end = psug + avail_commands_nr;
                    while (psug != psug_end && *psug != NULL) {
                        fprintf(stderr, "'%s'%s", *psug, (((psug + 1) == psug_end || *(psug + 1) == NULL) ? "?" : ", "));
                        psug++;
                    }
                    fprintf(stderr, " If not give 'zc help' a try.\n");
                } else {
                    fprintf(stderr, " Give 'zc help' a try.\n");
                }
            }

            if (avail_commands != NULL) {
                free(avail_commands);
            }

            if (suggestions != NULL) {
                free(suggestions);
            }

            return 1;
        }
    }
    return do_help();
}

int zc_help_help(void) {
    struct zc_exec_table_ctx *cmd = &g_command_table[0], *cmd_end = cmd + g_command_table_nr;
    fprintf(stdout, "use: zc help <topic>\n");
    fprintf(stdout, "\nThe available commands are:\n");
    while (cmd != cmd_end) {
        fprintf(stdout, "\t%s\n", cmd->cmd_name);
        cmd++;
    }
    return 0;
}
