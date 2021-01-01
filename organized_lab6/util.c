#include "util.h"

//Get the block details
int get_block(int dev, int blk, char *buf)
{
  off_t l = lseek(dev, (long)(blk*BLKSIZE), 0);
  if(l == -1)
  {
    printf("%s\n", strerror(errno));
    //assert(0);
  }
  read(dev, buf, BLKSIZE);
  return 1;
}

//Write the block details
int put_block(int dev, int blk, char *buf)
{
    off_t l = lseek(dev, (long)(blk*BLKSIZE), 0);
    if(l == -1)
    {
        //assert(0);
    }
    write(dev, buf, BLKSIZE);
    return 1;
}

//tokenize the pathname specified
char** tokenPath(char* pathname)
{
  int i = 0;
  char** name;
  char* tmp;
  name = (char**)malloc(sizeof(char*)*256);
  name[0] = strtok(pathname, "/");
  i = 1;
  while ((name[i] = strtok(NULL, "/")) != NULL) { i++;}
  name[i] = 0;
  i = 0;
  while(name[i])
  {
    tmp = (char*)malloc(sizeof(char)*strlen(name[i]));
    strcpy(tmp, name[i]);
    name[i] = tmp;
    i++;
  }
  return name;
}

//search direct block, return ino
int search(int dev, char *str, INODE *ip)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE], temp[256];

  for(i = 0; i < 12; i++)
  {
    //block empty
    if(ip->i_block[i] == 0){break;} 
    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strcmp(str, temp) == 0){ return dp->inode;}
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
  return 0;
}

//Get ino number
int getino(int dev, char *path)
{
  int ino = 0, i = 0;
  char **tokens;
  MINODE *mip = NULL;
  if(path && path[0])
  {
    //get tokenized pathname
    tokens = tokenPath(path);
  } 
  else
  {
    //no path means cwd
    ino = running->cwd->ino;
    return ino;
  }
  if(path[0] == '/')
  {
    ip = &(root->INODE);
    ino = root->ino;
  }
  else 
  {
    //start at cwd
    ip = &(running->cwd->INODE);
  }
  while(tokens[i])
  {
    //search from token
    ino = search(dev, tokens[i], ip);
    if(0 >= ino) 
    {
      if(mip)
      {
        iput(mip->dev, mip);
      }
      return -1;
    }
    if(mip)
    {
      iput(mip->dev, mip);
    }
    i++;
    if(tokens[i])
    {
      mip = iget(dev, ino);
      ip = &(mip->INODE);
    }
  }
  i = 0;
  while(tokens[i])
  {
    //free token for next
    free(tokens[i]);
    i++;
  }
  if(mip) 
  {
    iput(mip->dev, mip);
  }
  return ino;
}

//get minode pointer
MINODE *iget(int dev, unsigned int ino)
{
  int i = 0, blk, offset;
  char buf[BLKSIZE];
  MINODE *mip = NULL;
  for(i = 0; i < 64; i++)
  {
    //if inode in array, mip point to minode in array
    if(minode[i].refCount && minode[i].ino == ino && minode[i].dev == dev)
    {
      mip = &minode[i];
      minode[i].refCount++;
      return mip;
    }
  }

  // if inode doesn't exist, put inode disk to a free MINODE
  i = 0;
  while(minode[i].refCount > 0 && i < 64) { i++;}
  if(i == 64)
  {
    return 0;
  }
  blk = (ino-1)/8 + inode_start;
  offset = (ino-1)%8;
  get_block(dev, blk, buf);
  ip = (INODE *)buf + offset;
  //copy from virtual disk to array
  memcpy(&(minode[i].INODE), ip, sizeof(INODE)); 
  minode[i].dev = dev;
  minode[i].ino = ino;
  minode[i].refCount = 1;
  minode[i].dirty = 0;
  minode[i].mounted = 0;
  minode[i].mountPtr = NULL;
  return &minode[i];
}

//releases used minode
int iput(int dev, MINODE *mip)
{
  char buf[BLKSIZE];
  int blk, offset;
  INODE *tex;

  mip->refCount--;
  //check if mip is used
  if(mip->refCount > 0) {return 1;} 
  //check if mip is dirty
  if(mip->dirty == 0) {return 1;} 

  //write INODE to disk
  blk = (mip->ino-1)/8 + inode_start;
  offset = (mip->ino-1)%8;
  get_block(dev, blk, buf); 

  tex = (INODE*)buf + offset;
  memcpy(tex, &(mip->INODE), sizeof(INODE)); //minode to temp inode tex
  put_block(mip->dev, blk, buf);
  mip->refCount = 0;
  return 1;
}

//search either file or dir from ino
int search_ino(int dev, int ino, INODE *ip, char* temp)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE];

  for(i = 0; i < 12; i++)
  {
    if(ip->i_block[i] == 0)
    {
      break;
    }
    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;
    while(cp < buf+BLKSIZE)
    {
      if(ino == dp->inode)
      {
        strncpy(temp, dp->name, dp->name_len);
        return 1;
      }
    cp += dp->rec_len;
    dp = (DIR*)cp;
    }
  }
  return 0;
}

// Finding parent in the path specified
int find_parent(char *path)
{
  int j = 0;
  while(j < strlen(path))
  {
    if(path[j] == '/')
      return 1;
    j++;
  }
  return 0;
}

//Inode table
void inode_table(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, GDBLOCK, buf);
  gp = (GD*)buf;
  inode_start = gp->bg_inode_table;
  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
}

//Test the bit
int test_bit(char* buf, int i)
{
  int byt, offset;
  byt = i/8;
  offset = i%8;
  return (((*(buf+byt))>>offset)&1);
}

//Set the bit
int set_bit(char* buf, int i)
{
  int byt, offset;
  char temp;
  char *tempBuf;
  byt = i/8;
  offset = i%8;
  tempBuf = (buf+byt);
  temp = *tempBuf;
  temp |= (1<<offset);
  *tempBuf = temp;
  return 1;
}

//Clear the bit
int clear_bit(char* buf, int i)
{
  int byt, offset;
  char temp;
  char *tempBuf;
  byt = i/8;
  offset = i%8;
  tempBuf = (buf+byt);
  temp = *tempBuf;
  temp &= (~(1<<offset));
  *tempBuf = temp;
  return 1;
}

//decrements the number of free inodes
int dec_free_inodes(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER*)buf;
  sp->s_free_inodes_count -= 1; 
  put_block(dev, SUPERBLOCK, buf);
  get_block(dev, GDBLOCK, buf);
  gp = (GD*)buf;
  gp->bg_free_inodes_count -=1;
  put_block(dev, GDBLOCK, buf);
  return 1;
}

//increment the number of free inodes
int inc_free_inodes(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER*)buf;
  sp->s_free_inodes_count += 1;
  put_block(dev, SUPERBLOCK, buf);
  get_block(dev, GDBLOCK, buf);
  gp = (GD*)buf;
  gp->bg_free_inodes_count +=1;
  put_block(dev, GDBLOCK, buf);
  return 1;

}
//Decrementing free blocks available
int dec_free_blocks(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER*)buf;
  sp->s_free_blocks_count -= 1;
  put_block(dev, SUPERBLOCK, buf);
  get_block(dev, GDBLOCK, buf);
  gp = (GD*)buf;
  gp->bg_free_blocks_count -=1;
  put_block(dev, GDBLOCK, buf);
  return 1;
}

//Incrementing the free blocks available
int inc_free_blocks(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER*)buf;
  sp->s_free_blocks_count += 1;
  put_block(dev, SUPERBLOCK, buf);
  get_block(dev, GDBLOCK, buf);
  gp = (GD*)buf;
  gp->bg_free_blocks_count +=1;
  put_block(dev, GDBLOCK, buf);
  return 1;

}

//searching for directory name
int direcname(char *pathname, char buf[256])
{
  int i = 0;
  memset(buf, 0, 256);
  strcpy(buf, pathname);
  while(buf[i]) 
  {
    i++;
  }
  while(i >= 0)
  {
    if(buf[i] == '/') 
    {
      buf[i+1] = 0; //splitting the path based on /
      return 1;
    }
    i--;
  }
  buf[0] = 0;
  return 1;
}

int bsname(char *pathname, char *buf)
{
  int i = 0, j = 0;
  if(!pathname[0]) {return -1;}
  i = strlen(pathname);
  while(i >= 0 && pathname[i] != '/')
  {
    i--;
  }
  if(pathname[i] == '/')
  {
    i++;
    while(pathname[i])
    {
      buf[j++] = pathname[i++];//copy to buf
    }
    buf[j] = 0;
    return 1;
  }
  else 
  {
    strncpy(buf, pathname, 256);
  }
  return 1;
}

//allocate inode
int ialloc(int dev)
{
  int i;
  char buf[BLKSIZE];
  get_block(dev, imap, buf); 
  for (i=0; i < ninodes; i++){
    if (test_bit(buf, i)==0){//check if buf is used
      set_bit(buf, i);
      put_block(dev, imap, buf);
      dec_free_inodes(dev);//update
      return (i+1);             
    }
  }
  return 0;
}


//allocates block same as ino
int balloc(int dev)
{
  int i;
  char buf[BLKSIZE]; 

  get_block(dev, bmap, buf);

  for (i=0; i < BLKSIZE; i++){
    if (test_bit(buf, i)==0){
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      dec_free_blocks(dev);
      memset(buf, 0, BLKSIZE); //clear the buf to 0
      put_block(dev, i+1, buf);
      return (i+1);
    }
  }
  return 0;
}


//deallocate ino
void idalloc(int dev, int ino)
{
  char buf[BLKSIZE];

  get_block(dev, imap, buf);//get i to buf
  clear_bit(buf, ino-1);//clr
  put_block(dev, imap, buf);
  inc_free_inodes(dev);
}

void bdalloc(int dev, int ino)
{
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);  
  clear_bit(buf, ino-1);
  put_block(dev, bmap, buf);
  inc_free_blocks(dev);
}


//check if the dir is empty 
int is_dir_empty(MINODE *mip)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE], temp[256];
  for(i = 0; i < 12; i++)
  {
    if(ip->i_block[i] == 0) { return 0; }

    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strncmp(".", temp, 1) != 0 && strncmp("..", temp, 2) != 0) 
      {
        return 1;
      }
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
  return 0;
}

//Split pathname for chmod
int splitting_path(char *original, char *path1, char *path2)
{
  //split spaces in name path array
  char *temp;
  temp = strtok(original, " ");
  strcpy(path1, temp);
  temp = strtok(NULL, " ");
  if(temp == NULL)
  {
    return -1;
  }
  strcpy(path2, temp);
  return 1;
}

//find last minode
int find_last_block(MINODE *pip)
{
  int buf[256];
  int buf2[256];
  int i, j;

  if(pip->INODE.i_block[0] == 0) {return 0;}
  //direct
  for(i = 0; i < 12; i++)
  {
    if(pip->INODE.i_block[i] == 0)
    {
      return (pip->INODE.i_block[i-1]);
    }
  }
  if(pip->INODE.i_block[12] == 0) {return pip->INODE.i_block[i-1];}
  //check indirect
  get_block(dev, pip->INODE.i_block[12], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    //look for the free blocks
    if(buf[i] == 0) 
    {
      return buf[i-1];
    }
  }
  //check double indirect
  if(pip->INODE.i_block[13] == 0) {return buf[i-1];}
  memset(buf, 0, 256*4);
  get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0) 
    {
      return buf2[j-1];
    }
    if(buf[i])
    {
      get_block(pip->dev, buf[i], (char*)buf2);
      for(j = 0; j < 256; j++)
      {
        if(buf2[j] == 0) 
        {
          return buf2[j-1];
        }
      }
    }
  }
}

//adding last of minode
int add_last_block(MINODE *pip, int bnumber)
{
  int buf[256];
  int buf2[256];
  int i, j, newBlk, newBlk2;
  for(i = 0; i < 12; i++) //direct
  {
    if(pip->INODE.i_block[i] == 0) {pip->INODE.i_block[i] = bnumber; return 1;}
  }
  //indirect
  if(pip->INODE.i_block[12] == 0)
  {
    newBlk = balloc(pip->dev);
    pip->INODE.i_block[12] = newBlk;
    memset(buf, 0, 256*4);
    get_block(pip->dev, newBlk, (char*)buf);
    buf[0] = bnumber;
    put_block(pip->dev, newBlk, (char*)buf);
    return 1;
  }
  memset(buf, 0, 256*4);
  get_block(pip->dev, pip->INODE.i_block[12], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0) {buf[i] = bnumber; return 1;}
  }
  //double indirect
  if(pip->INODE.i_block[13] == 0) 
  {
    newBlk = balloc(pip->dev);
    pip->INODE.i_block[13] = newBlk;
    memset(buf, 0, 256*4);
    get_block(pip->dev, newBlk, (char*)buf);
    newBlk2 = balloc(pip->dev);
    buf[0] = newBlk2;
    put_block(pip->dev, newBlk, (char*)buf);
    memset(buf2, 0, 256*4);
    get_block(pip->dev, newBlk2, (char*)buf2);
    buf2[0] = bnumber;
    put_block(pip->dev, newBlk2, (char*)buf2);
    return 1;
  }
  memset(buf, 0, 256*4);
  get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0)
    {
      newBlk2 = balloc(pip->dev);
      buf[i] = newBlk2;
      put_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
      memset(buf2, 0, 256*4);
      get_block(pip->dev, newBlk2, (char*)buf2);
      buf2[0] = bnumber;
      put_block(pip->dev, newBlk2, (char*)buf2);
      return 1;
    }
    memset(buf2, 0, 256*4);
    get_block(pip->dev, buf[i], (char*)buf2);
    for(j = 0; j < 256; j++)
    {
      if(buf2[j] == 0) {buf2[j] = bnumber; return 1;}
    }
  }
  printf("Failed block to node\n");
  return -1;
}

int getinom(int *dev, char *name)
{
	int ino;
	char buf[256];
	char *str = buf;
	strncpy(buf, name, 256);

	*dev = running->cwd->dev;
	ino = running->cwd->ino;
	if (*str == '/') {
		str++;
		*dev = root->dev;
		ino = root->ino;
	}

	char *tok[256];
	int len = tokenize(str, "/", tok);

	MINODE *mip = iget(*dev, ino);
	if (mip == NULL)
		return 0;

	for (int i = 0; i < len; i++) {
		if ((mip->INODE.i_mode & 0xF000) != 0x4000) {
			// check if inode is a directory
			break;
		} else if (mip->ino == 2 && strcmp(tok[i], "..") == 0) {
			// going upstream out of mount point
			int i;
			for (i = 0; i < NMOUNT; i++) {
				if (MountTable[i].dev == mip->dev) {
					break;
				}
			}
			MINODE *m = MountTable[i].mounted_inode;

			// load the mount point
			iput(mip->dev, mip);
			mip = iget(m->dev, m->ino);

			// select parent of mount point
			*dev = mip->dev;
			ino = searchm(mip, "..");

			// load parent of mount point
			iput(mip->dev, mip);
			mip = iget(*dev, ino);
      ino = searchm(mip, tok[i]);
		} else if (ino == 0) {
			// find next inode number
			break;
		} else {
			// load next memory inode
			iput(mip->dev, mip);
			mip = iget(*dev, ino);

			if (mip->mounted) {
				// check if inode is mount point
				// if it is, then move into it
				*dev = mip->mountPtr->dev;
				ino = 2;

				iput(mip->dev, mip);
				mip = iget(*dev, ino);
			}
		}
	}

	if (mip)
		iput(mip->dev, mip);

	return ino;
}

int tokenize(char *path, char *delim, char *buf[])
{
	int n = 0;
	for (char *tok = strtok(path, delim); tok != NULL;
			tok = strtok(NULL, delim)) {
		buf[n++] = tok;
	}
	return n;
}

int searchm(MINODE *parent, char *name)
{
	int len = strlen(name);
	for (int i = 0; i < 12; i++) {
		char buf[BLOCK_SIZE];
		DIR *dp = (DIR *)buf;

		get_block(parent->dev, parent->INODE.i_block[i], buf);

		for (int j = 0; j < BLOCK_SIZE; j += dp->rec_len) {
			dp = (DIR *)(buf + j);
			if (dp->rec_len <= 0)
				return 0;
			if (len == dp->name_len && strncmp(name, dp->name, len) == 0)
				return dp->inode;
		}
	}
	return 0;
}

int has_perm(MINODE *mip, unsigned int perm)
{
	if (running->uid == 0)
		return 1;

	INODE *ip = &mip->INODE;
	unsigned int mode = ip->i_mode;

	if (ip->i_uid == running->uid) {
		mode = (mode & 00700) >> 6;
	} else if (ip->i_gid == running->gid) {
		mode = (mode & 00070) >> 3;
	} else {
		mode = (mode & 00007);
	}
	return ((mode & perm) == perm);
}

SUPER get_super(int dev)
{
	SUPER super;
	lseek(dev, (long)SUPERBLOCK*BLKSIZE, SEEK_SET);
	read(dev, &super, sizeof(super));
	return super;
}

GD get_group(int dev)
{
	GD group;
	lseek(dev, (long)GDBLOCK*BLKSIZE, SEEK_SET);
	read(dev, &group, sizeof(group));
	return group;
}

DIR * getDir(INODE * ip, int inum){
	char *cp;  char temp[256];
  char buf[BLKSIZE];
  DIR  *dp;
	int i = 0;

	// Convert the inumber to a INODE
	INODE* pip = 0;
	pip = getParentNode(ip, inum);
	if (pip == 0) {
		printf("Error traversing file system \n");
		return 0;
	}


    get_block(fd, pip->i_block[i], buf);
    cp = buf;
    dp = (DIR*)buf;

    while(cp < buf + BLOCK_SIZE){
    	strncpy(temp, dp->name, dp->name_len);
    	temp[dp->name_len] = 0;
    	if (dp->rec_len == 0) {
			return 0; }

    	if(dp->inode == inum ){
		//printf("Found the return dir: %s\n", dp->name );
		//iput(mip);
		return dp;
		      		}
		// move to the next DIR entry:
		cp += (dp->rec_len);   // advance cp by rec_len BYTEs
		dp = (DIR*)cp;
	}

}

MINODE *getParentMinode(INODE * ip, int inum){
	char *cp;  char temp[256];
  DIR  *dp;
  INODE *tempNode;
  char buf[BLKSIZE];

  printf("Get parent inode for inumber %d\n", inum);

  int i =0;

	if(inum < 2){
		printf("Encountered unknown inode %d\n", inum);
		return 0;
	}
	// Search the given inodes' i_blocks
	for (i = 0; i < 12; i++)
	{

	 	// Convert the i_block[0] to a buff
	 	get_block(fd, ip->i_block[i], buf);
	 	printf("the ip->i_block I get is: %d\n", ip->i_block[i]);
	 	cp = buf;
	 	// Convert the buff to a DIR
	 	dp = (DIR*)buf;

	 	while(cp < buf + BLOCK_SIZE){
    	strncpy(temp, dp->name, dp->name_len);

			temp[dp->name_len] = 0;


			printf("Searching in getParentIno name %s\n", temp);
			printf("Searched inode's rec length is: %d, "
					"its name_length is: %d\n ", dp->rec_len, dp->name_len );


			if (dp->rec_len == 0) {
				printf("encountered a record length of 0 for [%s]\n", temp);
				return 0;
			}

			// We have found the parent DIR
			if (strcmp(temp, "..") == 0 )
			{
				// Convert the inumber to a MINODE, be sure to put this away
				MINODE* mip = iget(dev,dp->inode);
				return mip;
			}
			// move to the next DIR entry:
				cp += (dp->rec_len);   // advance cp by rec_len BYTEs
				dp = (DIR*)cp;     // pull dp along to the next record
		}
	}
	return 0;
}

INODE *getParentNode(INODE * ip, int inum){
	char *cp;  char temp[256];
	char *buf2[BLKSIZE] = { 0 };
  char buf[BLKSIZE] = {0};
    DIR  *dp;
    INODE *tempNode;

    printf("Get parent inode for inumber %d\n", inum);

    int i =0;

	if(inum < 2){
		printf("Encountered unknown inode %d\n", inum);
		return 0;
	}
	// Search the given inodes' i_blocks
	for (i = 0; i < 12; i++)
	{

	  // Convert the i_block[0] to a buff
	  get_block(fd, ip->i_block[i], buf);
	  printf("the ip->i_block I get is: %d\n", ip->i_block[i]);
	  cp = buf;
	  // Convert the buff to a DIR
	  dp = (DIR*)buf;
   	while(cp < buf + BLOCK_SIZE){
     	strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			printf("Searching in getParentIno name %s\n", temp);
			printf("Searched inode's rec length is: %d, "
					"its name_length is: %d\n ", dp->rec_len, dp->name_len );


			if (dp->rec_len == 0) { 
				printf("encountered a record length of 0 for [%s]\n", temp); 
				return 0;
			}
      GD g = get_group(fd);
      int bg_inode_table = g.bg_inode_table;

			// We have found the parent DIR
			if (strcmp(temp, "..") == 0 )
			{
				int cur_blk = (dp->inode - 1) / 8 + bg_inode_table;
				int cur_offset = (dp->inode - 1) % 8;

				get_block(fd, cur_blk, buf2);
				// Convert the inumber to a MINODE, be sure to put this away
				INODE *pip = (INODE *)buf2 + cur_offset;
				return pip;
			}
			// move to the next DIR entry:
				cp += (dp->rec_len);   // advance cp by rec_len BYTEs
				dp = (DIR*)cp;     // pull dp along to the next record
		}
	}
	return 0;
}

