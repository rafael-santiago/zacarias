#ifndef ZACARIAS_CMD_UTILS_H
#define ZACARIAS_CMD_UTILS_H 1

#include <stdlib.h>

void del_scr_line(void);

char prompt(const char *question, const char *options, const size_t options_size);

#endif