#include <defs/types.h>

static struct cdev_ctx g_cdev_data;

struct cdev_ctx *g_cdev(void) {
    return &g_cdev_data;
}
