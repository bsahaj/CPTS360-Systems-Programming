#include "level2.h"

int my_write(int fd, char *buf, int nbytes)
{
  MINODE *mip; 
  OFT *oftp;
  int count, lblk, start, blk, dblk, remain;
  int ibuf[256], dbuf[256];
  char writeBuf[BLKSIZE], *cp, *cq = buf;
  count = 0;
  oftp = running->fd[fd];
  mip = oftp->inodePtr;
  while(nbytes)
  {
    lblk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    
    //convert logic to phys
    //direct blocks
    if(lblk < 12 ) 
    {
      if(mip->INODE.i_block[lblk]==0){
        mip->INODE.i_block[lblk] = balloc(mip->dev);
      }
      blk = mip->INODE.i_block[lblk];
    }
    //indirect blocks
    else if(lblk >= 12 && lblk < 256 + 12) 
    {
      if(mip->INODE.i_block[12]==0){
        mip->INODE.i_block[12] = balloc(mip->dev);
        memset(ibuf,0,256*4);
      }
      get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
      blk = ibuf[lblk - 12];
      if (blk==0){
        mip->INODE.i_block[lblk] = balloc(mip->dev);
        ibuf[lblk - 12] =mip->INODE.i_block[lblk];
      }
    }
    //double indirect blocks
    else 
    {
      memset(ibuf, 0, 256*4);
      get_block(mip->dev, mip->INODE.i_block[13], (char  *)dbuf);
      lblk -= (12 + 256);
      dblk = dbuf[lblk / 256];
      get_block(mip->dev, dblk, (char *)dbuf);
      blk = dbuf[lblk % 256];
    }
    memset(writeBuf,0,BLKSIZE);
    //read to buf
    get_block(mip->dev, blk, writeBuf);
    cp = writeBuf + start;
    remain = BLKSIZE - start;
    
    //copy
    // printf("cq = %s\n",cq);
    // while (remain){               // write as much as remain allows  
    //   *cp++ = *cq++;              // cq points at buf[ ]
    //   nbytes--; remain--;         // dec counts
    //   oftp->offset++;             // advance offset
    //   if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
    //   {
    //     mip->INODE.i_size++;    // inc file size (if offset > fileSize)
    //   }
    //   if (nbytes <= 0) 
    //   {
    //     break;     // if already nbytes, break
    //   }
    // }
    if(remain < nbytes)
    {
      strncpy(cp, cq, remain);
      count += remain;
      nbytes -= remain;
      running->fd[fd]->offset += remain;
      //check offset
      if(running->fd[fd]->offset > mip->INODE.i_size)
      {
        mip->INODE.i_size += remain;
      }
      remain -= remain;
    }
    else
    {
      strncpy(cp, cq, nbytes);
      count += nbytes;
      remain -= nbytes;
      running->fd[fd]->offset += nbytes;
      if(running->fd[fd]->offset > mip->INODE.i_size)
      {
        mip->INODE.i_size += nbytes;
      }
      nbytes -= nbytes;
    }
    put_block(mip->dev, blk, writeBuf);
    mip->dirty = 1;
    printf("Wrote %d chars into file.\n", count);
  }
  return count;
}

int write_file(char *pathname)
{
  int fd;
  char writeMe[BLKSIZE];
  fd = atoi(pathname);
  if(fd < 0 || fd > 9)
  {
    printf("No File descriptor\n");
    return -1;
  }
  if(running->fd[fd] == NULL)
  {
    printf("No file descriptor \n");
    return -1;
  }

  //check mode
  if(running->fd[fd]->mode == 0)
  {
    printf("File in read mode\n");
    return -1;
  }

  printf("Write: ");
  fgets(writeMe, BLKSIZE, stdin);
  writeMe[strlen(writeMe) -1] = 0;
  printf("writeme = %s",writeMe);
  if(writeMe[0] == 0)
  {
    return 0;
  }
  return my_write(fd, writeMe, strlen(writeMe));
}

int copy_file(char *source, char *target)
{
  char src[BLKSIZE];
  char dest[BLKSIZE];
  if(!strcmp(source, ""))
  {
    printf("no source specified\n");
    return 0;
  }
  strcpy(src, source);
  if(!strcmp(target, ""))
  {
    printf("no target specified\n");
    return 0;
  }
  strcpy(dest, target);

  //my_touch(dest);

  strcat(src, " R");
  strcat(dest, " W");

  int fd = open_file(src);
  int gd = open_file(dest);
  char buf[BLKSIZE];
  int n;

  if(fd == -1)
  {
    printf("invalid file descriptor");
    return 0;
  }
  while((n = my_read(fd, buf, BLKSIZE)))
  {
    my_write(gd, buf, n);
  }

  close_file(fd);
  close_file(gd);
  return 1;
}

int my_copy(char *pathname)
{
  char destination[256], source[256];
  splitting_path(pathname, source, destination);
  return copy_file(source, destination);
  
}