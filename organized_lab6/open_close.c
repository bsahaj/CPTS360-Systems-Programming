#include "level2.h"

//Opening file based on pathname
int open_file(char *pathname)
{
  char pathfile[256], modes[256];
  int mode, ino, i;
  MINODE *mip;
  OFT *oftp;
    
  if(splitting_path(pathname, pathfile, modes) <= 0) 
  {
    return -1;
  }
  if(strcmp("R", modes) == 0)
  {
    mode = 0;
  }
  else if(strcmp("W", modes) == 0)
  {
    mode = 1;
  }
  else if(strcmp("RW", modes) == 0)
  {
    mode = 2;
  }
  else if(strcmp("APPEND", modes) == 0)
  {
    mode = 3;
  }
  else
  {
    return -1;
  }

  //get minode
  ino = getino(dev, pathfile);
  if(ino <= 0)
  {
    //create file when path doesn't exist 
    my_creat(pathfile);
  }
  ino = getino(dev,pathfile);
  if(ino <= 0)
  {
    printf("Error while opening\n");
    return -1;
  }
  mip = iget(dev, ino);
  if(!S_ISREG(mip->INODE.i_mode))
  {
    printf("Not a regular file, probably directory.\n");
    iput(mip->dev, mip);
    return -1;
  }
  for(i = 0; i < 10; i++)
  {
    if(running->fd[i] != NULL)
    {
      if(running->fd[i]->inodePtr == mip)
      {
        if(running->fd[i]->mode != 0 || mode != 0)
        {
          printf("File in fd.\n");
          iput(mip->dev, mip);
          return -1;
        }
      }
    }
  }

  //allocate opft
  oftp = (OFT *)malloc(sizeof(OFT));
  oftp->mode = mode;
  oftp->refCount = 1;
  oftp->inodePtr = mip;
  //set offset 
  switch(mode)
  {
    case 0: oftp->offset = 0;
            printf("File opened for read\n");
            my_touch(pathfile);
            break;
    case 1: my_truncate(oftp->inodePtr);
            oftp->offset = 0;
            printf("File open for write\n");
            my_touch(pathfile);
            break;
    case 2: oftp->offset = 0;
            printf("File open for rw\n");
            my_touch(pathfile);
            break;
    case 3: oftp->offset = mip->INODE.i_size;
            printf("File open for append\n");
            my_touch(pathfile);
            break;
    default: printf("Invalid\n");
             iput(mip->dev, mip);
             free(oftp);
             return -1;
                //break;
  }
    
  //check file pointer
  i = 0;
  while(running->fd[i] != NULL && i < 10)
  {
    i++;
  }
  if(i == 10) //fd full
  {
    iput(mip->dev, mip);
    free(oftp);
    return -1;
  }
  running->fd[i] = oftp;
  if(mode != 0) 
  {
    mip->dirty = 1;
  }
  printf("i = %d\n", i);
  extfd = i;
  return i;
}

int close_file(int fd)
{
  MINODE *mip;
  OFT *oftp;
  fd = extfd;
  if(fd == 0)// no fd
  {
    printf("No file descriptor\n");
    return -1;
  }
  printf("fd = %d\n", fd);
  if(fd < 0 || fd > NFD)
  {
    printf("Fd invalid\n");
    return -1;
  }
  if(running->fd[fd] == NULL)
  {
    printf("File not open\n");
    return -1;
  }

  //close
  oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;
  if(oftp->refCount > 0)
  {
    return 0;
  }
  mip = oftp->inodePtr;
  iput(mip->dev, mip);
  free(oftp);

  printf("\nFile closed.\n");
  return 1;
}

// int my_close(char *path)
// {
//   char *command, *parameter;
//   splitting_path(path,command, parameter);
//   if(strcmp(parameter, "") == 0)
//   {
//     printf("no file decriptor\n");
//   }
//   int fd = atoi(parameter);
//   return close_file(fd);
// }