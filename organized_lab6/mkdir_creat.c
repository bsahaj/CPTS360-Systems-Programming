#include "level1.h"

int my_mkdir(char *pathname)
{
  int dev1, ino, r;
  char parent[256], child[256], origPathname[512];
  MINODE *mip;
  memset(parent, 0, 256);
  memset(child, 0, 256);
  memset(origPathname, 0, 512);

  strcpy(origPathname, pathname);
  //check root or dir
  if(!strcmp(pathname,"")){
    printf("Missing one or more operands\n"); 
    return -1;
  }
  if(pathname[0] == '/') 
  {
    dev1 = root->dev; 
  }
  else 
  {
    dev1 = running->cwd->dev; 
  }

  direcname(pathname, parent);
  bsname(origPathname, child);
  ino = getino(dev1, parent);
  if(ino <= 0)
  {
    printf("Invalid directory name\n");
    return -1;
  }
  mip = iget(dev1, ino);

  //check if dir exists
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Oops, Not a directory.\n");
    iput(dev1, mip);
    return -1;
  }
  //check if copy
  ino = search(dev1, child, &(mip->INODE));
  if(ino > 0)
  {
    printf("Dang it, Directory exists.\n");
    iput(mip->dev, mip);
    return -1;
  }
  r = rmkdir(mip, child);
  //release inode
  iput(mip->dev, mip);
  return r;
}

int my_creat(char* pathname)
{
  int dev1, ino, r;
  char parent[256];
  char child[256];
  MINODE *mip;
  memset(parent, 0, 256);
  memset(child, 0, 256);
  if(!strcmp(pathname,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  //start at root
  if(pathname[0] == '/') 
  {
    dev1 = root->dev;
  }
  //else cwd
  else 
  {
    dev1 = running->cwd->dev;
  } 

  direcname(pathname, parent);
  bsname(pathname, child);

  ino = getino(dev1, parent);
  if(ino <= 0)
  {
    printf("Invalid filename.\n");
    return -1;
  }
  mip = iget(dev1, ino);
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory.\n");
    iput(dev1, mip);
    return -1;
  }
  ino = search(dev1, child, &(mip->INODE));
  if(ino > 0)
  {
    printf("File already exists.\n");
    iput(mip->dev, mip);
    return -1;
  }
  r = creat_file(mip, child);
  iput(mip->dev, mip);
  return r;
}

int creat_file(MINODE *pip, char child[256])
{
  int inumber, bnumber, req_len, need_len, newRec, i;
  MINODE *mip;
  char *cp;
  char buf[BLKSIZE];

  //allocate ino
  inumber = ialloc(pip->dev);
  mip = iget(pip->dev, inumber);

  //Write contents
  mip->INODE.i_mode = 0x81A4;
  mip->INODE.i_uid = running->uid;
  mip->INODE.i_gid = running->gid;
  mip->INODE.i_size = 0;
  mip->INODE.i_links_count = 1;
  mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
  mip->INODE.i_blocks = 0;
  mip->dirty = 1;
  for(i = 0; i <15; i++)
  {
    mip->INODE.i_block[i] = 0;
  }
  iput(mip->dev, mip);
  //name
  memset(buf, 0, BLKSIZE);
  need_len = 4*((8+strlen(child)+3)/4);
  bnumber = find_last_block(pip);

  get_block(pip->dev, bnumber, buf);
  dp = (DIR*)buf;
  cp = buf;
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  req_len = 4*((8+dp->name_len+3)/4);
  if(dp->rec_len - req_len >= need_len)
  {
    newRec = dp->rec_len - req_len;
    dp->rec_len = req_len;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else 
  {
    //else allocate block
    bnumber = balloc(pip->dev);
    dp = (DIR*)buf;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    add_last_block(pip, bnumber);
  }
  
  //putback
  put_block(pip->dev, bnumber, buf);
  pip->dirty = 1;
  memset(buf, 0, BLKSIZE);
  search_ino(pip->dev, pip->ino, &running->cwd->INODE, buf);
  my_touch(buf);
  return 1;
}