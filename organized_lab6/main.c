#include "level1.h"
#include "level2.h"
#include "level3.h"

int fs_init()
{
  proc[0].uid = 0;
  proc[0].cwd = 0;
  proc[1].uid = 1;
  proc[1].cwd = 0;

  running = &(proc[0]);
  readQueue = &(proc[1]);
  int i = 0;
  for(i = 0; i < 64; i++)
  {
    minode[i].refCount = 0;
    minode[i].ino = 0;
  }
  for(i = 0; i < 10; i++) 
  {
    MountTable[i].dev = 0;
  }
  root = 0;
}

int mount_root(char *diskname)
{
  char buf[BLKSIZE];
  printf("mounting root\n");
  dev = open(diskname, O_RDWR);

  if (dev < 0){
    printf("open %s failed\n", diskname);
    exit(1);
  }
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER*)buf;
  if (SUPER_MAGIC != sp->s_magic)
  {
    printf("Not an EXT2 file sytem\n");
    exit(0);
  }
  //Set inode_start to start
  //Set root inode
  //set cwd proc to root inode

  inode_table(dev); 
  ninodes = sp->s_inodes_count;
  root = iget(dev, ROOT_INODE); 
  proc[0].cwd = iget(dev, ROOT_INODE); 
  proc[1].cwd = iget(dev, ROOT_INODE);
  MountTable[0].mounted_inode = root;
  MountTable[0].ninodes = ninodes;
  MountTable[0].nblocks = sp->s_blocks_count;
  MountTable[0].dev = dev;
  strncpy(MountTable[0].name, diskname, 256);
  return dev;
}

char *cmds[] = {"ls", "cd", "pwd", "mkdir", "chmod",  "creat", "rmdir", "link", "unlink", "symlink", "touch", "open", "close", "write", "read", "stat", "lseek", "cat", "cp", "pfd", "mount", "cs", "quit"};
static int (*fptr[23])(char*) = {(int (*)())my_ls, my_chdir, my_pwd, my_mkdir, my_chmod, my_creat, my_rmdir, my_link, my_unlink, my_symlink, my_touch, open_file, close_file, write_file, read_file, my_stat, my_lseek, my_cat, my_copy, my_pfd, my_mount, my_switch, quit};

int main(int argc, char *argv[])
{
  char line[128], cmnd[64], param[64];
  char *diskname = "diskimage";
  if(argc > 1)
  {
    diskname = argv[1];
  }
  fs_init();
  mount_root(diskname);

  while(1)
  {
    memset(pathname, 0, 128);
    memset(param, 0, 64);
    memset(cmnd, 0, 64);

    printf("P%d running:\n", running->pid);
    printf("Input command from following:");
    printf("[ls | pwd | cd | mkdir | rmdir | creat | link | unlink | symlink | cat | cp | open | read | write | close | mount | unmount | cs]: ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;
    if(line[0] == 0)
    {
      continue;
    }
    sscanf(line, "%s %s %s", cmnd, pathname, param);
    if(param[0] != 0)
    {
      strcat(pathname, " ");
      strcat(pathname, param);
    }
    for(int i = 0; i<23; i++)
    {
      if(strcmp(cmds[i], cmnd) == 0)
      {
        fptr[i](pathname);
        continue;
      }
    }
    putchar(10);
  }
}