#ifndef ZACARIAS_DEV_LINUX_CDEV_OPEN_H
#define ZACARIAS_DEV_LINUX_CDEV_OPEN_H 1

#include <linux/fs.h>

int cdev_open(struct inode *ip, struct file *fp);

#endif