#include "level1.h"


int my_chdir(char* pathname)
{
  MINODE *mip;
  unsigned int ino;
  if (strlen(pathname) == 0 || strcmp(pathname, "/") == 0){
    ino = root->ino;
  } 
  else { 
    ino = getino(dev, pathname);
  }
  //load inode
  mip = iget(dev, ino); 
  //check if dir
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error: it is not a directory\n");
    iput(dev, mip);
    return 0;
  }
  iput(dev, running->cwd); 
  running->cwd = mip;   //new minode to mip
}

int my_ls(char *path)
{

  unsigned long ino;
  MINODE *mip;
  int device = running->cwd->dev;
  char *child;

  if(path[0] == 0)// print cwd
  {
    mip = iget(device, running->cwd->ino);
    ls_dir(device, mip);
  }
  else
  {
    if(path[0] == '/')
    {
      device = root->dev;
    }
    ino = getino(device, path);
    //if the path doesn' exist?
    if(ino < 1)
    {
      return 1;
    }
    mip = iget(device, ino);

    //check if it is directory?
    if(((mip->INODE.i_mode) & 0040000)!= 0040000)
    {
      //get basename
      if(find_parent(path))
      {
        child = basename(pathname);
      }
      else
      {
        child = (char *)malloc((strlen(pathname) + 1) * sizeof(char));
        strcpy(child, path);
      }
      //print file
      ls_file(mip, child);
      iput(mip->dev, mip);
      return 0;
    }
    //print dir
    ls_dir(device, mip);
  }

  //iput(mip->dev, mip);
  return 0;
}

void ls_file(MINODE *mip, char *namebuf)
{
  INODE* pip = &mip->INODE;
  u16 mode = pip->i_mode;
  time_t val = pip->i_ctime;
  char *mtime = ctime(&val);
  mtime[strlen(mtime) - 1] = '\0';
  u16 type = mode & 0xF000;
  switch(type)
  {
    case 0x4000:
        printf("d");
        break;
    case 0x8000:
        printf("-");
        break;
    case 0xA000:
        printf("l");
        break;
    default:
        printf("-");
        break;
  }
  printf( (mode & S_IRUSR) ? "r" : "-");
  printf( (mode & S_IWUSR) ? "w" : "-");
  printf( (mode & S_IXUSR) ? "x" : "-");
  printf( (mode & S_IRGRP) ? "r" : "-");
  printf( (mode & S_IWGRP) ? "w" : "-");
  printf( (mode & S_IXGRP) ? "x" : "-");
  printf( (mode & S_IROTH) ? "r" : "-");
  printf( (mode & S_IWOTH) ? "w" : "-");
  printf( (mode & S_IXOTH) ? "x" : "-");
  printf("%4d%4d%4d  %s%8d    %s", pip->i_links_count, pip->i_gid, pip->i_uid, mtime, pip->i_size, namebuf);
  // if this is a symlink file, show the file it points to
  if((mode & 0120000) == 0120000)
  {
    printf(" => %s\n",(char *)(mip->INODE.i_block));
  }
  else
  {
    printf("\n");
  }
}

void ls_dir(int devicename, MINODE *mp)
{
  char buf[BLKSIZE], namebuf[256], *cp;
  DIR *dp;
  int i, ino;
  MINODE *temp;
  //print from direct
  for(i = 0; i < 12; i++)
  {
    if(mp->INODE.i_block[i])
    {
      get_block(devicename, mp->INODE.i_block[i], buf);
      cp = buf;
      dp = (DIR *)buf;

      while(cp < &buf[BLKSIZE])
      {

        strncpy(namebuf, dp->name, dp->name_len);
        namebuf[dp->name_len] = 0;

        ino = dp->inode;
        temp = iget(devicename, ino);

        ls_file(temp, namebuf);

        cp+=dp->rec_len;
        dp = (DIR *)cp;
        iput(temp->dev,temp);
      }
    }
  }
}


int my_pwd(char *pathname)
{
  char *pathnameprint;
  printf("cwd = ");
  rpwd(running->cwd);
  printf("\n");
}

int rpwd(MINODE *wd)
{
  int ino = 0;
  MINODE *next = NULL;
  char temp[256];
  //check root
  if(wd == root) 
  {
    printf("/");
    return 1;
  }
  //parent minode
  ino = search(dev, "..", &(wd->INODE));
  //ino = getinom(&dev, "..");
  if(ino <= 0){return -1;}

  next = iget(dev, ino); 

  if(!next){return -1;}

  // recursive till it reaches root
  rpwd(next); 
  memset(temp, 0, 256);
  //find dir name
  search_ino(next->dev, wd->ino, &(next->INODE), temp);
  printf("%s/", temp);
  //put back
  iput(next->dev, next); 
  return 1;
}