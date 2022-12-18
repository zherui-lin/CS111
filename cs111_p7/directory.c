#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NUM_DIRENTS_PER_BLOCK (DISKIMG_SECTOR_SIZE/sizeof(struct direntv6)) // 512 / 16 = 32
#define MAX_COMPONENT_LENGTH sizeof(dirEnt->d_name) // 14

int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
        // You need to implement this
        if (strlen(name) > MAX_COMPONENT_LENGTH) {
                fprintf(stderr, "Name length exceeds maximum\n");
                return -1;
        }
        struct inode in;
        inode_iget(fs, dirinumber, &in);
        if ((in.i_mode & IFMT) != IFDIR) {
                fprintf(stderr, "Inode %d does not represent a directory\n", dirinumber);
                return -1;
        }
        int dirBlockCount = inode_getsize(&in) / DISKIMG_SECTOR_SIZE + (inode_getsize(&in) % DISKIMG_SECTOR_SIZE != 0);
        for (int i = 0; i < dirBlockCount; i++) {
                int blockNum = inode_indexlookup(fs, &in, i);
                struct direntv6 buffer[NUM_DIRENTS_PER_BLOCK];
                int readLen = diskimg_readsector(fs->dfd, blockNum, buffer);
                if (readLen < 0) {
                        fprintf(stderr, "Can't read the directory block at physical block number %d\n", blockNum);
                        return -1;
                }
                for (int j = 0; j < (int) NUM_DIRENTS_PER_BLOCK; j++) {
                        if(strncmp(name, buffer[j].d_name, MAX_COMPONENT_LENGTH) == 0) {
                                *dirEnt = buffer[j];
                                return 0;
                        }
                }
        }
        fprintf(stderr, "Can't find %s in the directory\n", name);
        return -1;
}
