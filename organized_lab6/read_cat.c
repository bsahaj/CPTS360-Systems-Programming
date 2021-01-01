#include "level2.h"

int my_read(int fd, char *buf, int nbytes)
{
  MINODE *mip; 
  OFT *oftp;
  int count = 0;
  int lbk, blk, startByte, remain;
  int avil;
  int *ip;

  int indirect_blk;
  int indirect_off;

  char readbuf[1024];


  oftp = running->fd[fd];
  //printf("mode = %d\n", oftp->mode);
  if(!oftp) {
    printf("file for write\n");
    return -1;
  }

  mip = oftp->inodePtr;
  //calculate byte to read
  avil = mip->INODE.i_size - oftp->offset;
  char *cq = buf;

  while(nbytes && avil)
  {
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;

    //direct block
    if(lbk < 12)
    {
      blk = mip->INODE.i_block[lbk];
      //printf("direct\n");
    }
    //indirect blocks
    else if(lbk >= 12 && lbk < 256 + 12)
    {
      get_block(mip->dev, mip->INODE.i_block[12], readbuf);

      ip = (int *)readbuf + lbk - 12;
      blk = *ip;
      //printf("indirect\n");

    }
    //double indirect
    else
    {
      get_block(mip->dev, mip->INODE.i_block[13], readbuf);

      indirect_blk = (lbk - 256 - 12) / 256;
      indirect_off = (lbk - 256 - 12) % 256;
      printf("blk = %d, ofset = %d\n", indirect_blk, indirect_off);
      getchar();

      ip = (int *)readbuf + indirect_blk;
      getchar();
      get_block(mip->dev, *ip, readbuf);
      getchar();
      ip = (int *)readbuf + indirect_off;
      blk = *ip;
      getchar();
      //printf("double indirect\n");
    }

    //getblock to buf
    get_block(mip->dev, blk, readbuf);
    printf("readbuf = %d and %s\nstartbyte = %d\n", readbuf, readbuf, startByte);
    char *cp = readbuf + startByte;

    remain = BLKSIZE - startByte;
    int temp =remain ^ ((avil ^ remain) & -(avil < remain));
    int temp2 =nbytes ^ ((temp ^ nbytes) & -(temp < nbytes));
    //temp2 = 1;
    //printf("minimum bytes = %d\n", temp2);
    //check available and remaining
    while(remain > 0)
    {
      // printf("avail = %d, remain = %d\n", avil, remain);
      // printf("temp2 = %d\n", temp2);
      strncpy(cq, cp, temp2);
      //*cq++ = *cp++;
      oftp->offset += temp2;
      count += temp2;
      avil -= temp2;
      nbytes -= temp2;
      remain -= temp2;
      // printf("avail = %d, remain = %d\n", avil, remain);
      if(nbytes <= 0 || avil <= 0)
        break;
    }
  }
  //printf("myread: read %d char from file descriptor %d\n", count, fd);
  return count;
}

int read_file(char *pathname)
{
  char secondPath[256], path[256];
  splitting_path(pathname, path, secondPath);
  //convert bytes to int
  int nbytes = atoi(secondPath), actual = 0;
  int fd = 0;
  char buf[nbytes + 1];

  strcpy(buf, "");
  //check fd
  if (!strcmp(pathname, ""))
  {
    printf("No File descriptor\n");
    return 0;
  }
  //convert fd to int
  fd = atoi(pathname);
  if (!strcmp(secondPath, ""))
  {
    printf("No byte\n");
    return 0;
  }

  //return byte read
  actual = my_read(fd, buf, nbytes);

  if (actual == -1)
  {
    strcpy(secondPath, "");
    return 0;
  }

  buf[actual] = '\0';
  printf("actual = %d buf = %s\n", actual, buf);
  return actual;
}

int my_cat(char *pathname)
{
  char mybuf[BLKSIZE];
  int n;
  char* temppath;
  strcpy(temppath, pathname);
  strcat(temppath," R");
  int fd = open_file(temppath);
  while((n = my_read(fd, mybuf, BLKSIZE)))
  {
    printf("\ncoming here though\n");
    mybuf[n]= 0;
    //printf("%s",mybuf);
    fputs(mybuf, stdout);
  }
  close_file(fd);
  return 1;
}