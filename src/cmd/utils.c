#include <cmd/utils.h>
#include <string.h>
#include <stdio.h>

void del_scr_line(void) {
    fprintf(stdout, "\r                                                      \r");
    fflush(stdout);
}

char prompt(const char *question, const char *options, const size_t options_size) {
    char opt[2] = { 0 };

    if (question == NULL || options == NULL || options_size == 0) {
        return 0;
    }

    do {
        fprintf(stdout, "\r%s", question);
        fscanf(stdin, "%c%c", &opt[0], &opt[1]);
        opt[1] = 0;
    } while(strstr(options, opt) == NULL);

    return opt[0];
}
