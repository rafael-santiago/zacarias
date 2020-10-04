#include <cmd/version.h>
#include <cmd/types.h>
#include <stdio.h>

int zc_version(void) {
    fprintf(stdout, "zc version %s\n", ZC_VERSION);
    return 0;
}

int zc_version_help(void) {
    fprintf(stdout, "use: zc version\n");
    return 0;
}
