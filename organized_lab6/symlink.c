#include "level1.h"

int my_symlink(char *pathname)
{
  char oldname[256], newname[256];
  int ino;
  MINODE *mip;

  if(splitting_path(pathname, oldname, newname) <= 0) 
  {
    return -1;
  }
  if((ino = getino(dev, oldname))<= 0)
  {
    printf("File doesn't exist\n");
    return -1;
  }
  //create file
  my_creat(newname);
  if(0 >= (ino = getino(dev, newname)))
  {
    return -1;
  }
  mip = iget(dev, ino);
  mip->INODE.i_mode = 0120000; //symlink
  mip->dirty = 1;

  strcpy((char*)mip->INODE.i_block, oldname);
  iput(mip->dev, mip);
}
