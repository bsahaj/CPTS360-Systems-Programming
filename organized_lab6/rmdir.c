#include "level1.h"

int rmkdir(MINODE *pip, char child[256])
{
  int inumber, bnumber, req_len, need_len, newRec, i;
  MINODE *mip;
  char *cp;
  char buf[BLKSIZE];

  //allocate
  inumber = ialloc(pip->dev);
  bnumber = balloc(pip->dev);

  //get inode to mem
  mip = iget(pip->dev, inumber);

  //makedir update
  mip->INODE.i_mode = 0x41ED; //directory mode
  mip->INODE.i_uid = running->uid;
  mip->INODE.i_gid = running->gid;
  mip->INODE.i_size = BLKSIZE;
  mip->INODE.i_links_count = 2;

  mip->INODE.i_mtime = time(0L);
  mip->INODE.i_atime = mip->INODE.i_ctime; 
  mip->INODE.i_ctime = mip->INODE.i_mtime;

  mip->INODE.i_blocks = 2;
  mip->dirty = 1;
  for(i = 0; i <15; i++) 
  {
    mip->INODE.i_block[i] = 0; 
  }
  mip->INODE.i_block[0] = bnumber;
  //release inode
  iput(mip->dev, mip);

  // current dir and previous dir
  dp = (DIR*)buf;
  dp->inode = inumber;
  strncpy(dp->name, ".", 1);
  dp->name_len = 1;
  dp->rec_len = 12;

  cp = buf + 12;
  dp = (DIR*)cp;
  dp->inode = pip->ino;
  dp->name_len = 2;
  strncpy(dp->name, "..", 2);
  dp->rec_len = BLKSIZE - 12;

  //put block to mem
  put_block(pip->dev, bnumber, buf);
  memset(buf, 0, BLKSIZE);
  need_len = 4*((8+strlen(child)+3)/4);
  //find available block
  bnumber = find_last_block(pip);
  //get block buffer
  get_block(pip->dev, bnumber, buf);

  cp = buf;
  dp = (DIR*)cp;
  //traverse to the last item in the block
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  req_len = 4*((8+dp->name_len+3)/4);

  //check if enough room
  if(dp->rec_len - req_len >= need_len)
  {
    //update
    newRec = dp->rec_len - req_len;
    dp->rec_len = req_len;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else //if no room
  {
    //allocate new block and add data
    bnumber = balloc(pip->dev);
    dp = (DIR*)buf;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    add_last_block(pip, bnumber);
  }
  //put block to disk
  put_block(pip->dev, bnumber, buf);
  pip->dirty = 1;
  pip->INODE.i_links_count++;
  memset(buf, 0, BLKSIZE);
  //check parent
  search_ino(pip->dev, pip->ino, &running->cwd->INODE, buf);
  //update time using touch
  my_touch(buf);
  return 1;
}

int my_rmdir(char *pathname)
{
  int ino, i;
  char parent[256], child[256], origPathname[512];
  MINODE *pip = NULL;
  MINODE *mip = NULL;
  strcpy(origPathname, pathname);
  //checkdirname
  if(!pathname || !pathname[0])
  {
    printf("Error: No directory given\n");
    return -1;
  }
  else
  {
    ino = getino(dev, pathname);
  }
  if(0 >= ino)
  {
    printf("Invalid.\n");
    return -1;
  }
  //load mem
  mip = iget(dev, ino);
  if(!has_perm(mip, 2))
  {
    printf("rmdir failed: wrong permissions\n");
    return -1;
  }
  //check if it is a dir
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory.\n");
    iput(mip->dev, mip);
    return -1;
  }
  //check if in dir
  if(mip->refCount > 1)
  {
    printf("In Directory.\n");
    return -1;
  }
  //check if not empty
  if(mip->INODE.i_links_count > 2)
  {
    printf("Directory not empty.(dir in dir)\n");
    iput(mip->dev, mip);
    return -1;
  }
  //Check data blocks if file exist
  if(is_dir_empty(mip) != 0)
  {
    printf("Directory not empty.(file in dir)\n");
    iput(mip->dev, mip);
    return -1;
  }
  for(i = 0; i < 12; i++)
  {
    if(mip->INODE.i_block[i] != 0)
    {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
  }
  idalloc(mip->dev, mip->ino);

  direcname(origPathname, parent);
  bsname(origPathname, child);
  //update
  ino = getino(mip->dev, parent);
  pip = iget(mip->dev, ino);
  iput(mip->dev, mip);
  rm_child(pip, child);
  pip->INODE.i_links_count--;
  my_touch(parent);
  pip->dirty = 1;
  iput(pip->dev, pip);
  return 1;
}

int rm_child(MINODE *pip, char *child)
{
  int i, size, foundflag = 0;
  char *cp, *cp2;
  DIR *dp, *dp2, *dpPrev;
  char buf[BLKSIZE], buf2[BLKSIZE], temp[256];

  memset(buf2, 0, BLKSIZE);
  //check direct
  for(i = 0; i < 12; i++)
  {
    if(pip->INODE.i_block[i] == 0) { return 0; }
    //load to mem
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    dp2 = (DIR *)buf;
    dpPrev = (DIR *)buf;
    cp = buf;
    cp2 = buf;

    while(cp < buf+BLKSIZE && !foundflag)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strcmp(child, temp) == 0)
      {
        //if only child
        if(cp == buf && dp->rec_len == BLKSIZE)
        {
          //deallocate
          bdalloc(pip->dev, pip->INODE.i_block[i]);
          pip->INODE.i_block[i] = 0;
          pip->INODE.i_blocks--;
          foundflag = 1;
        }
        //else delete child
        else
        {
          while((dp2->rec_len + cp2) < buf+BLKSIZE)
          {
            dpPrev = dp2;
            cp2 += dp2->rec_len;
            dp2 = (DIR*)cp2;
          }
          if(dp2 == dp)
          {
            dpPrev->rec_len += dp->rec_len;
            foundflag = 1;
          }
          else
          {
            size = ((buf + BLKSIZE) - (cp + dp->rec_len));
            dp2->rec_len += dp->rec_len;
            //copy
            memmove(cp, (cp + dp->rec_len), size);
            dpPrev = (DIR*)cp;
            memset(temp, 0, 256);
            strncpy(temp, dpPrev->name, dpPrev->name_len);
            foundflag = 1;
          }
        }
      }
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
    if(foundflag)
    {
      //putback to disk
      put_block(pip->dev, pip->INODE.i_block[i], buf);
      return 1;
    }
  }
  return -1;
}