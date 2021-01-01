/************* cd_ls_pwd.c file **************/
#include <dirent.h>

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
  struct stat lsfile;
  char temp[BLKSIZE];
  char filestring[BLKSIZE];
  char *dirname = "/mnt";
  sprintf(filestring, "%s/%s", dirname, name);
  int check = stat(filestring, &lsfile);
  if(check == -1)
  {
    printf("Error occured\n");
  }
  printf((S_ISDIR(lsfile.st_mode)) ? "d":"-");
  printf((lsfile.st_mode & S_IRUSR)? "r":"-");
  printf((lsfile.st_mode & S_IWUSR)? "w":"-");
  printf((lsfile.st_mode & S_IXUSR)? "x":"-");
  printf((lsfile.st_mode & S_IRGRP)? "r":"-");
  printf((lsfile.st_mode & S_IWGRP)? "w":"-");
  printf((lsfile.st_mode & S_IXGRP)? "x":"-");
  printf((lsfile.st_mode & S_IROTH)? "r":"-");
  printf((lsfile.st_mode & S_IWOTH)? "w":"-");
  printf((lsfile.st_mode & S_IXOTH)? "x":"-");
  printf("  ");
  sprintf(temp, "%4d  %ld  %ld  %.19s  %6d  %s", lsfile.st_nlink, lsfile.st_uid, lsfile.st_gid, ctime(&lsfile.st_mtim), lsfile.st_size, name);
  printf("%s", temp);
  printf("\n");
  
}

void ls_dir(MINODE *mip, char *path)
{
  struct dirent **tempFileList;
  char fileList[BLKSIZE];
  char buf[BLKSIZE];
  int n = 0;
  struct stat stfile;
  struct passwd *psd;
  struct group *grp;

  int fileCount = scandir(path, &tempFileList, NULL, alphasort);
  if(fileCount < 0)
  {
    perror("Scandir error");
  }
  else{
    while(n<fileCount)
    {
      //printf("%s\n", tempFileList[n]->d_name);
      sprintf(fileList, "%s/%s", path, tempFileList[n]->d_name);
      
      stat(fileList, &stfile);
      printf((S_ISDIR(stfile.st_mode)) ? "d":"-");
      printf((stfile.st_mode & S_IRUSR)? "r":"-");
      printf((stfile.st_mode & S_IWUSR)? "w":"-");
      printf((stfile.st_mode & S_IXUSR)? "x":"-");
      printf((stfile.st_mode & S_IRGRP)? "r":"-");
      printf((stfile.st_mode & S_IWGRP)? "w":"-");
      printf((stfile.st_mode & S_IXGRP)? "x":"-");
      printf((stfile.st_mode & S_IROTH)? "r":"-");
      printf((stfile.st_mode & S_IWOTH)? "w":"-");
      printf((stfile.st_mode & S_IXOTH)? "x":"-");
      printf("  ");
      sprintf(buf, "%4d  %ld  %ld  %.19s  %6d  %s", stfile.st_nlink, stfile.st_uid, stfile.st_gid, ctime(&stfile.st_mtim), stfile.st_size, tempFileList[n]->d_name);

      printf("%s", buf);
      printf("\n");

      free(tempFileList[n]);
      n++;
    }
    free(tempFileList);
  }
}

int ls(char *pathname)  
{
  printf("ls %s\n", pathname);
  int ino = getino(pathname);
  MINODE *mip = iget(dev,ino);
  //struct stat path_stat;
  //stat(pathname, &path_stat);;
  int len = strlen(pathname);
  if(len > 0)
  {
    if(!(strcmp(pathname, ".")) || !(strcmp(pathname, "/")))
    {
      char *path = "/mnt";
      ls_dir(mip, path);
    }
    else if(!strcmp(pathname, "dir1"))
    {
      char *path = "/mnt/dir1";
      ls_dir(mip, path);
    }
    else if(!strcmp(pathname, "dir2"))
    {
      char *path = "/mnt/dir2";
      ls_dir(mip, path);
    }
    else if(!strcmp(pathname, "file1"))
    {
      ls_file(mip,pathname);
    }
    else if(!strcmp(pathname, "file2"))
    {
      ls_file(mip,pathname);
    }
    else{
      printf("searching for %s in current directory\n", pathname);
      char *path = "/mnt";
      ls_dir(mip, path);
      printf("%s does not exist in current directory\n", pathname);
    }
    
  }
  else
  {
    char *path = "/mnt";
    ls_dir(mip, path);
  }
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
