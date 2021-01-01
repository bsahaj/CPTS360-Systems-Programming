#include "level1.h"
/*    Miscellaneous functions    */

int my_touch (char* name)
{
  int ino;
  MINODE *mip;

  ino = getino(dev, name);
  if(ino <= 0)
  {
    //create file if input unavailable
    my_creat(name);
    return 1;
  }
  mip = iget(dev, ino);
  //update time based on ctime
  mip->INODE.i_ctime =(unsigned)time(NULL);
  mip->INODE.i_atime = mip->INODE.i_mtime;
  mip->INODE.i_mtime = mip->INODE.i_ctime;
  mip->dirty = 1;
  iput(mip->dev, mip);
  return 1;
}

int my_chmod(char* pathname)
{
  char nMode[256];
  char path[256];
  int ino, newMode, i;
  MINODE* mip;

  //check if pathname is empty?
  if(!strcmp(pathname,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  //split based on space
  if(splitting_path(pathname, nMode, path) <= 0) 
  {
    return -1; 
  }
  //convert mode to ul
  newMode = strtoul(nMode, NULL, 8);
  ino = getino(dev, path);
  if(ino <= 0)
  {
    //if file is not there, quit
    return -1;
  }

  //save the inode into memory
  mip = iget(dev, ino);
  
  // performing bitwise operation
  i = ~0x1FF;
  mip->INODE.i_mode &= i;
  mip->INODE.i_mode |= newMode;
  mip->dirty = 1;
  iput(dev, mip);
  return 1;
}

int rm(MINODE *mip)
{
  int i, j;
  int buf[256], buf2[256];

  if(!S_ISLNK(mip->INODE.i_mode))
  {
    for(i = 0; i < 12; i++)
    {
      if(mip->INODE.i_block[i] != 0)
      {
        //deallocate block 
        bdalloc(mip->dev, mip->INODE.i_block[i]);
      }
    }
    if(mip->INODE.i_block[12] != 0)
    {
      memset(buf, 0, 256*4); //clearing the buffer array 
      get_block(mip->dev, mip->INODE.i_block[12], (char*)buf);
      for(i = 0; i < 256; i++)
      {
        if(buf[i] != 0) 
        {
          bdalloc(mip->dev, buf[i]);
        }
      }
      bdalloc(mip->dev, mip->INODE.i_block[12]);
    }
    if(mip->INODE.i_block[13] != 0)
    {
      memset(buf, 0, 256*4);
      get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
      for(i = 0; i < 256; i++)
      {
        if(buf[i] != 0)
        {
          memset(buf2, 0, 256*4);
          get_block(mip->dev, buf[i], (char*)buf2);
          for(j = 0; j < 256; j++)
          {
            if(buf2[j] != 0) {bdalloc(mip->dev, buf2[j]);}
          }
          bdalloc(mip->dev, buf[i]);
        }
      }
      bdalloc(mip->dev, mip->INODE.i_block[13]);
    }
  }
  idalloc(mip->dev, mip->ino);
  return 1;
}

int quit()
{
  int j = 0;
  for(j = 0; j < 64; j++)
  {
    if(minode[j].refCount > 0)
    {
      if(minode[j].dirty != 0)
      {
        minode[j].refCount = 1;
        iput(dev, &minode[j]);
      }
    }
  }
  exit(0);
}


int my_stat (char *pathname)
{
  int ino;
  MINODE *mip;
  // check if pathname is empty
  if(!strcmp(pathname,""))
  {
    printf("Missing operand");
    return -1;
  }
  ino = getino(dev, pathname);

  mip = iget(dev, ino);
  //print each of the fields
  time_t val = mip->INODE.i_ctime;
  char *mtime = ctime(&val);
 printf("*********  stat  *********\n");
 printf("dev=%d ino=%d mod=%x\n",mip->dev,ino,mip->INODE.i_mode);
 printf("uid=%d gid=%d nlink=%d\n",mip->INODE.i_uid,mip->INODE.i_gid,mip->INODE.i_links_count);
 printf("size=%d time=%s",mip->INODE.i_size,mtime);
  return 1;
}