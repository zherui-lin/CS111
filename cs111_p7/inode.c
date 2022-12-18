#include <stdio.h>
#include <assert.h>

#include "inode.h"
#include "diskimg.h"
#include <stdbool.h>

#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE/sizeof(struct inode)) // 512 / 32 = 16
#define NUM_BLOCK_NUMS_PER_BLOCK (DISKIMG_SECTOR_SIZE/sizeof(uint16_t)) // 512 / 2 = 256
#define MAX_SINGLE_INDIRECT (7)
#define MAX_SINGLE_INDIRECT_FILE_BLOCK_INDEX (MAX_SINGLE_INDIRECT*NUM_BLOCK_NUMS_PER_BLOCK) // 7 * 256
#define MAX_SINGLE_INDIRECT_FILE_SIZE (MAX_SINGLE_INDIRECT_FILE_BLOCK_INDEX*DISKIMG_SECTOR_SIZE) // 7 * 256 * 512

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    // You need to implement this
    int inum = inumber - ROOT_INUMBER; // Important!!!
    int inodeBlockNum = inum / INODES_PER_BLOCK + INODE_START_SECTOR;
    int inodeOffset = inum % INODES_PER_BLOCK;
    struct inode buffer[INODES_PER_BLOCK];
    int readLen = diskimg_readsector(fs->dfd, inodeBlockNum, buffer);
    if (readLen < 0) {
        fprintf(stderr, "Can't read the block where inode %d is located\n", inumber);
        return -1;
    }
    *inp = buffer[inodeOffset];
    if (inp == NULL) {
        fprintf(stderr, "Can't find inode %d\n", inumber);
        return -1;
    }
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int fileBlockIndex) {
    // You need to implement this
    if ((inp->i_mode & IALLOC) == 0) {
        fprintf(stderr, "Inode unallocated\n");
        return -1;
    }
    int res;
    if ((inp->i_mode & ILARG) == 0) {
        res = inp->i_addr[fileBlockIndex];
    } else if (fileBlockIndex < (int) MAX_SINGLE_INDIRECT_FILE_BLOCK_INDEX) {
        int singleBlockIndex = fileBlockIndex / NUM_BLOCK_NUMS_PER_BLOCK;
        int fileBlockOffset = fileBlockIndex % NUM_BLOCK_NUMS_PER_BLOCK;
        uint16_t singleIndirect[NUM_BLOCK_NUMS_PER_BLOCK];
        int readLen = diskimg_readsector(fs->dfd, inp->i_addr[singleBlockIndex], singleIndirect);
        if (readLen < 0) {
            fprintf(stderr, "Can't read the single indirect block where file block index %d is located\n", fileBlockIndex);
            return -1;
        }
        res = singleIndirect[fileBlockOffset];
    } else {
        int extraFileBlockIndex = fileBlockIndex - MAX_SINGLE_INDIRECT_FILE_BLOCK_INDEX;
        int singleBlockIndex = extraFileBlockIndex / NUM_BLOCK_NUMS_PER_BLOCK;
        if (singleBlockIndex >= (int) NUM_BLOCK_NUMS_PER_BLOCK) {
            fprintf(stderr, "File block index %d exceeds maximum\n", fileBlockIndex);
            return -1;
        }
        int fileBlockOffset = extraFileBlockIndex % NUM_BLOCK_NUMS_PER_BLOCK;
        uint16_t doubleIndirect[NUM_BLOCK_NUMS_PER_BLOCK];
        int readDoubleLen = diskimg_readsector(fs->dfd, inp->i_addr[7], doubleIndirect);
        if (readDoubleLen < 0) {
            fprintf(stderr, "Can't read the double indirect block\n");
            return -1;
        }
        uint16_t singleIndirect[NUM_BLOCK_NUMS_PER_BLOCK];
        int readSingleLen = diskimg_readsector(fs->dfd, doubleIndirect[singleBlockIndex], singleIndirect);
        if (readSingleLen < 0) {
            fprintf(stderr, "Can't read the single indirect block where file block index %d is located\n", fileBlockIndex);
            return -1;
        }
        res = singleIndirect[fileBlockOffset];
    }
    if (res == 0) {
        fprintf(stderr, "Can't find the physical block number at file block index %d\n", fileBlockIndex);
        return -1;
    }
    return res;
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1);
}
