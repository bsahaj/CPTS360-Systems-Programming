#include "level3.h"

int allocate_mount(void)
{
	for(int i=0;i<NMOUNT;i++)
	{
		if(MountTable[i].dev == 0)
		{
			return i;
		}
	}
	return -1;
}

int get_mount(char *file)
{
	for (int i = 0; i < NMOUNT; i++)
		if (strcmp(MountTable[i].name, file) == 0)
			return i;
	return -1;
}

int is_mount_busy(int dev)
{
	for (int i = 0; i < NMINODES; i++) {
		if (minode[i].dev == dev && minode[i].ino != 2) {
			printf("inode: %d\n", minode[i].ino);
			return 1;
		}
	}
	return 0;
}

void print_mount()
{
	printf("currently mounted disks are:\n");
	for (int i = 0; i < NMOUNT; i++) {
		if (MountTable[i].dev) {
			printf("Name:%s\t,  ", MountTable[i].name);
			printf("Mount:%s\t,  ", MountTable[i].mount_name);
			printf("Device:%d\n", MountTable[i].dev);
		}
	}
}

void mount_disk(char *file, char *path)
{
	int fd = -1, md;
	int dev, ino;
	MOUNT *mntptr;
	MINODE *mip = NULL;
	SUPER s;

    if (path[0] != '/') 
	{
		printf("mount: failed: may only mount relative to root\n");
	}
	else if (get_mount(file) >= 0) {
		printf("mount: failed: '%s' is already mounted\n", file);
	} 
	else if (fd = open(file, O_RDWR), fd < 0) {
		printf("mount: failed: could not open '%s'\n", file);
	} 
	else if (s = get_super(fd), s.s_magic != SUPER_MAGIC) {
		printf("mount: failed: filesystem is not EXT2\n");
	} 
	else if (ino = getinom(&dev, path), ino == 0) {
		printf("mount: failed: mount point does not exist\n");
	} 
	else if (mip = iget(dev, ino), mip == NULL) {
		printf("mount: error: inode not found\n");
	} 
	else if (md = allocate_mount(), md < 0) {
		printf("mount: failed: mount table is full\n");
	} 
	else {
		GD g = get_group(fd);

		MOUNT *m = &MountTable[md];
		m->mounted_inode = mip;
		m->dev = fd;
		m->nblocks = s.s_blocks_count;
		m->ninodes = s.s_inodes_count;
		m->bmap = g.bg_block_bitmap;
		m->imap = g.bg_inode_bitmap;
		m->iblock = g.bg_inode_table;
		strncpy(m->name, file, 64);
		strncpy(m->mount_name, path, 64);

		mip->mounted = 1;
		mip->mountPtr = m;

		return; // exit here so the mount stays open
	}
	if (fd >= 0)
	{
		close(fd);
	}
}

void umount_disk(char *file)
{
	int md;
	md = get_mount(file);
	if (md < 0) 
    {
		printf("umount: failed: '%s' is not mounted\n", file);
	} 
    else if (is_mount_busy(MountTable[md].dev)) 
    {
		printf("umount: failed: mount is busy\n");
	} 
    else {
		MOUNT *m = &MountTable[md];

		iput(m->dev, m->mounted_inode);
		close(m->dev);

		m->mounted_inode = 0;
		m->dev = 0;
		m->nblocks = 0;
		m->ninodes = 0;
		m->bmap = 0;
		m->imap = 0;
		m->iblock = 0;
		memset(m->name, 0, 64);
		memset(m->mount_name, 0, 64);
	}
}

/*
 * pswitch:
 * @uid: The UID of the process to switch to.
 *
 * Switches the running process to the given one.
 */
void pswitch(void)
{
	if (running == &proc[0]) {
		printf("inside here, current uid=%d\n",proc[0].uid);
		running = &(proc[1]);
        readQueue = &(proc[0]);
		printf("inside here, changed uid=%d\n",proc[1].uid);
	} else {
		printf("inside here, current uid=%d\n",proc[1].uid);
		running = &(proc[0]);
        readQueue = &(proc[1]);
		printf("inside here, changed uid=%d\n",proc[0].uid);
	}
}

void my_mount(char *pathname)
{
	char diskPath[256], path[256];
    splitting_path(pathname, diskPath, path);
	print_mount();
	if((strcmp(diskPath, "") == 0) || (strcmp(path, "") == 0))
	{
		printf("Insufficient parameters\n");
	}
	printf("file is %s\n", diskPath);
	printf("path is %s\n", path);
	mount_disk(diskPath, path);
}

void my_switch(void)
{
	pswitch();
}