#include "level1.h"

int my_link(char* pathname)
{
  char old_file[256], new_file[256], parent[256], child[256], buf[BLKSIZE];
  int ino, ino2, bnumber, need_len, req_len, newRec;
  MINODE *mip, *mip2;
  char *cp;
  DIR *dp;
  if(splitting_path(pathname, old_file, new_file) <= 0) 
  {
    return -1;
  }
  
  ino = getino(dev, old_file);
  //checking for file existance
  if(ino <= 0)
  {
    printf("File does not exist\n");
    return -1;
  }
  mip = iget(dev, ino);
  //check isreg
  if(!S_ISREG(mip->INODE.i_mode))
  {
    printf("Not regular file type\n");
    iput(mip->dev, mip);
    return -1;
  }
  direcname(new_file, parent);
  bsname(new_file, child);
  //checking the second file existence
  ino2 = getino(mip->dev, parent);
  if(ino2 <= 0)
  {
    printf("File doesn not exist\n");
    iput(mip->dev, mip);
    return -1;
  }
  mip2 = iget(mip->dev, ino2);
  //check if the directory is parent
  if(!S_ISDIR(mip2->INODE.i_mode))
  {
    printf("This is not a directory\n");
    iput(mip->dev, mip);
    iput(mip2->dev, mip2);
    return -1;
  }
  //check if filename used already
  ino2 = search(mip2->dev, child, &(mip2->INODE));
  if(ino2 > 0)
  {
    printf("File already there on disk\n");
    iput(mip->dev, mip);
    iput(mip2->dev, mip2);
    return -1;
  }

  memset(buf, 0, BLKSIZE);
  need_len = 4*((8+strlen(child)+3)/4);
  bnumber = find_last_block(mip2);
  //check if there is enough space to create one more file on the disk.
  get_block(mip2->dev, bnumber, buf);
  dp = (DIR*)buf;
  cp = buf;
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  req_len = 4*((8+dp->name_len+3)/4);
  if((dp->rec_len - req_len) >= need_len)
  {
    newRec = dp->rec_len - req_len;
    dp->rec_len = req_len;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = ino;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else 
  {
    //allocate space if there.
    bnumber = balloc(mip2->dev);
    dp = (DIR*)buf;
    dp->inode = ino;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    add_last_block(mip2, bnumber);
  }

  put_block(mip2->dev, bnumber, buf);
  mip->dirty = 1;
  mip->INODE.i_links_count++;
  memset(buf, 0, BLKSIZE);
  search_ino(mip2->dev, mip2->ino, &running->cwd->INODE, buf);
  iput(mip->dev, mip);
  iput(mip2->dev, mip2);
  return 1;
}

int my_unlink(char *pathname)
{
  char parent[256], child[256], old_path[512];
  int ino, ino2;
  MINODE *mip, *mip2;
  strcpy(old_path, pathname);
  //check file
  ino = getino(dev, pathname);
  if(ino <= 0)
  {
    printf("File doesn't exist\n");
    return -1;
  }
  mip = iget(dev, ino);
  if(!has_perm(mip, 2))
  {
    printf("unlink failed: wrong permissions\n");
    return -1;
  }

  //check if it is reg file or link
  if(!S_ISREG(mip->INODE.i_mode) && !S_ISLNK(mip->INODE.i_mode))
  {
    printf("File not LNK or REG\n");
    iput(mip->dev, mip);
    return -1;
  }
  mip->INODE.i_links_count--;
  if(mip->INODE.i_links_count <= 0)
  {
    rm(mip);
  }
  direcname(old_path, parent);
  bsname(old_path, child);
  ino2 = getino(mip->dev, parent);

  //check if there is child
  if(ino2 == -1 || ino2 == 0) {return -1;}
  mip2 = iget(mip->dev, ino2);
  rm_child(mip2, child);
}