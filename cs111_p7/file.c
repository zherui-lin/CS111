#include <stdio.h>
#include <assert.h>

#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int fileBlockIndex, void *buf) {
    // You need to implement this
    struct inode in;
    inode_iget(fs, inumber, &in);
    int blockNum = inode_indexlookup(fs, &in, fileBlockIndex);
    int readLen = diskimg_readsector(fs->dfd, blockNum, buf);
    if (readLen < 0) {
        fprintf(stderr, "Can't read the file block at file block index %d\n", fileBlockIndex);
        return -1;
    }
    if (fileBlockIndex < inode_getsize(&in) / DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    }
    return inode_getsize(&in) % DISKIMG_SECTOR_SIZE;
}
