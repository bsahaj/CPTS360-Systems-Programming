/************* cd_ls_pwd.c file **************/
#include <dirent.h>


void pathParse(char *path)
{
  char *s = "/";
  int counter = 0;
  char *dname = strdup(path);
  char *bname = strdup(path);
  parentdir = dirname(dname);
  base = basename(bname);
  //char *ch = strchr(parentdir, s);
  // if(ch != NULL)
  // {
  //   pathParse(parentdir);
  // }

}

int chdir(char *pathname)   
{
  int ino = 0;
  MINODE *mip;

  if((strlen(pathname) == 0) || (strcmp(pathname, "/") == 0))
  {
    ino = root->ino;
  }
  else{
      ino = getino(pathname);
  }
   mip = iget(dev, ino);
   if(!S_ISDIR(mip->INODE.i_mode))
   {
     printf("Error: not a directory");
     iput(mip);
     return 0;
   }
    iput(running->cwd);
    running->cwd = mip;
}

void ls_file(MINODE *mip, char *name)
{
  printf("careful!!!! in error zone\n");
  char *temp;
  INODE *lsp = &mip->INODE;
  int mode = lsp->i_mode;
  int type = mode & 0xF000;
  time_t attime = lsp->i_ctime;
  char *mtime = ctime(&attime);
  mtime[strlen(mtime)-1] = '\0';
  printf("place #2\n");
  
  
  printf((S_ISDIR(mode)) ? "d":"-");
  printf((mode & S_IRUSR)? "r":"-");
  printf((mode & S_IWUSR)? "w":"-");
  printf("ikade dobbindi antava\n");
  printf((mode & S_IXUSR)? "x":"-");
  printf((mode & S_IRGRP)? "r":"-");
  printf((mode & S_IWGRP)? "w":"-");
  printf((mode & S_IXGRP)? "x":"-");
  printf("kaade.. mari ikada??");
  printf((mode & S_IROTH)? "r":"-");
  printf((mode & S_IWOTH)? "w":"-");
  printf((mode & S_IXOTH)? "x":"-");
  printf("  ");
  sprintf(temp, "%4d  %ld  %ld  %.19s  %6d  %s", lsp->i_links_count, lsp->i_gid, lsp->i_uid, mtime, lsp->i_size, name);
  printf("%s", temp);
  printf("\n");
  
}

void ls_dir(MINODE *mip, int devname)
{
  printf("inside ls_dir with devname- %d\n", devname);
  char buffer[BLKSIZE];
  char temp[256];
  char *pch;
  DIRI *pdir;
  int ino = 0;
  MINODE *t;

  if(mip->INODE.i_block[0])
  {
     get_block(devname, mip->INODE.i_block[0], buffer);
    pdir = (DIRI*)buffer;
    pch = buffer;
      
    while(pch < buffer + BLKSIZE)
    {
      strncpy(temp, pdir->name, pdir->name_len);
      printf("inside this stupid loop\n");
      temp[pdir->name_len] = 0;

      //ino = pdir->inode;
      //t = iget(devname, ino);
      printf("in error zone\n");

      printf("%d %d %d %s \n", pdir->inode, dp->rec_len, dp->name_len, temp);
      printf("probably this might be it, but lets see\n");

      pch += pdir->rec_len;
      pdir = (DIRI*)pch;
    }
  }

  
}

int ls(char *pathname)  
{
  printf("ls %s\n", pathname);
  int ino = getino(pathname);
  MINODE *mip;
  char *childpath;
  int devno = running->cwd->dev;

  if(strcmp(pathname, "") == 0)
  {
    mip = iget(devno,running->cwd->ino);
    printf("%d-dev %d-ino number\n",devno, running->cwd->ino);
    ls_dir(mip, devno);
  }
  else{
    if(!strcmp(pathname, "/"))
    {
      devno = root->dev;
    }
    ino = getino(pathname);
    if(ino < 1)
    {
      return 1;
    }
    mip = iget(devno, ino);
    if(!S_ISDIR(mip->INODE.i_mode))
    {
      if(findparent(pathname))
      {
        childpath = basename(pathname);
      }
      else
      {
        childpath = (char*)malloc((strlen(pathname)+ 1)* sizeof(char));
        strcpy(childpath, pathname);
      }
      ls_file(mip, childpath);
      iput(mip);
      return 0;
    }
    ls_dir(mip, devno);
  }
  iput(mip);
  return 0;
}

int rpwd(MINODE *wd)
{
  int ino = 0;
  MINODE *later;
  char *pwpath[BLKSIZE];
  memset(pwpath, 0, BLKSIZE);
  if (wd == root){
    printf("/\n");
    return 1;
  }
  ino = search(wd, "..");
  if(ino <= 0)
  {
    return -1;
  }
  later = iget(dev,ino);
  if(!later)
  {
    return -1;
  }
  rpwd(later);
  findino(later, later->dev, wd->ino, pwpath);
  printf("%s/",pwpath);
  iput(later);
  return 1;
}

int pwd(MINODE *wd)
{
  printf("cwd = ");
  rpwd(running->cwd);
  printf("\n");
  
}
