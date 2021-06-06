#include <cmd/utils.h>
#include <stdio.h>

void del_scr_line(void) {
    fprintf(stdout, "\r                                                      \r");
    fflush(stdout);
}

