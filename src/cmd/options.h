#ifndef ZACARIAS_CMD_OPTIONS_H
#define ZACARIAS_CMD_OPTIONS_H 1

void zc_set_argc_argv(const int argc, char **argv);

char *zc_get_option(const char *option, char *default_value);

int zc_get_bool_option(const char *option, const int default_value);

char *zc_get_command(void);

char *zc_get_subcommand(void);

#endif
