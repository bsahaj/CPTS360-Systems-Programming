#ifndef LEVEL3_H
#define LEVEL3_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"
#include "type.h"
#include "level1.h"
#include "level2.h"

int allocate_mount(void);
int get_mount(char *file);
int is_mount_busy(int dev);
void mount_disk(char *filesys, char *path);
void umount_disk(char *filesys);
void my_mount(char *pathname);
void pswitch(void);
void my_switch(void);
#endif

