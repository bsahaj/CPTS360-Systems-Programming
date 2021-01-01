#include "level2.h"
#include "level1.h"

void my_pfd()
{
  int i=0;
  char mode[128] = {0};
  OFT *fd = 0;
  printf("fd\tmode\toffset\tinode\n");
  printf("-----------------------------\n");
  for(i=0;i<NFD; i++)
  {
    fd = running->fd[i];
    if(fd == 0)
    {
      continue;
    }
    if(fd->refCount >= 1)
    {
      if(fd->mode == 0)
      {
        strcpy(mode, "R");
      }
      else if(fd->mode == 1)
      {
        strcpy(mode, "W");
      }
      else if(fd->mode == 2)
      {
        strcpy(mode, "RDWR");
      }
      else if(fd->mode == 3)
      {
        strcpy(mode, "A");
      }
      else
      {
        printf("opened in invalid mode");
      }
      printf("%d\t%d\t%.4d\t[%d, %d]\n", i, fd->mode, fd->offset, fd->inodePtr->dev, fd->inodePtr->ino);
    }
  }
}

int my_truncate(MINODE *mip)
{
  int buf[256];
  int buf2[256];
  int i, j;

  if(mip == NULL)
  {
    printf("Error: No file returned");
    return -1;
  }
  for(i=0;i<12;i++)
  {
    if(mip->INODE.i_block[i] != 0)
    {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
  }
  if(mip->INODE.i_block[12] == 0)
  {
    return -1;
  }
  get_block(dev, mip->INODE.i_block[12], (char*)buf);
  for(i=0; i<256;i++)
  {
    if(buf[i] != 0)
    {
      bdalloc(mip->dev, buf[i]);
    }
  }
  bdalloc(mip->dev, mip->INODE.i_block[12]);
  if(mip->INODE.i_block[13] == 0)
  {
    return 1;
  }
  memset(buf, 0, 256*4);
  get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
  for(i=0;i<256;i++)
  {
    if(buf[i])
    {
      get_block(mip->dev, buf[i], (char*)buf2);
      for(j=0;j<256;j++)
      {
       if(buf2[j] != 0)
       {
          bdalloc(mip->dev, buf2[j]);
       }
      }
      bdalloc(mip->dev, buf[i]);
    }
            
  }
  bdalloc(mip->dev, mip->INODE.i_block[13]);
  mip->INODE.i_mtime = time(0L);
  mip->INODE.i_atime = mip->INODE.i_mtime;
  mip->INODE.i_size = 0;
  mip->dirty = 1;
  return 1;
}

int file_lseek(int fd, int position)
{
  OFT *oftp;
  oftp = running->fd[fd];
  int max = oftp->inodePtr->INODE.i_size - 1;
  int min = 0;

  if(position > max || position < min)
  {
    printf("Out of bounds.\n");
    return -1;
  }
  int originalPosition = oftp->offset;
  oftp->offset = position;
  return originalPosition;
}

int my_lseek(char * pathname)
{
  char secondPath[256], path[256];
  splitting_path(pathname, path, secondPath);
  //convert bytes to int
  int nbytes = atoi(secondPath);
  int fd = 0;

  //check fd
  if (!strcmp(pathname, ""))
  {
    printf("No FD\n");
    return 0;
  }
  //convert fd to int
  fd = atoi(pathname);
  if (!strcmp(secondPath, ""))
  {
    printf("No byte\n");
    return 0;
  }
  return file_lseek(fd, nbytes);
}
