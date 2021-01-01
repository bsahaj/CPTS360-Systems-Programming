#ifndef LEVEL2_H
#define LEVEL2_H

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

int my_truncate(MINODE *mip);
int open_file(char *pathname);
int close_file(int fptr);
int write_file(char *pathname);
int read_file(char *path);
int copy_file(char *src, char* dest);
int file_lseek(int fptr, int dist);
int my_write(int fptr, char *buf, int nbytes);
int my_read(int fptr, char *buf, int nbytes);
int my_lseek(char *pathname);
int my_cat(char *pathname);
int my_copy(char *pathname);
void my_pfd();
int my_close(char *path);

int extfd;
#endif