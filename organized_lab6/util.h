#ifndef UTIL_H
#define UTIL_H

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
#include "type.h"

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int search(int dev, char *str, INODE *ip);
int getino(int dev, char *path);
int getinom(int *dev, char *path);
MINODE *iget(int dev, unsigned int ino);
int iput(int dev, MINODE *mip);
int search_ino(int dev, int ino, INODE *ip, char *temp);
int find_parent(char *pathn);
void inode_table(int dev);
int test_bit(char* buf, int i);
int set_bit(char* buf, int i);
int clear_bit(char* buf, int i);
int dec_free_inodes(int dev);
int inc_free_inodes(int dev);
int dec_free_blocks(int dev);
int inc_free_blocks(int dev);
int direcname(char *pathname, char buf[256]);
int bsname(char *pathname, char buf[256]);
int ialloc(int dev);
int balloc(int dev);
void idalloc(int dev, int ino);
void bdalloc(int dev, int ino);
int is_dir_empty(MINODE *mip);
int splitting_path(char* original, char* path1, char* path2);
int tokenize(char *path, char *delim, char *buf[]);
int find_last_block(MINODE *pip);
int add_last_block(MINODE *pip, int bnumber);
int searchm(MINODE *parent, char *name);
int has_perm(MINODE *mip,unsigned int perm);
GD get_group(int dev);
SUPER get_super(int dev);
DIR * getDir(INODE * ip, int inum);
MINODE *getParentMinode(INODE * ip, int inum);
INODE *getParentNode(INODE * ip, int inum);
#endif