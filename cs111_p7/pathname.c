
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    // You need to implement this
    if (*pathname != '/') {
        fprintf(stderr, "%s does not represent an absolute path\n", pathname);
        return -1;
    }
    char *curr;
    char buffer[strlen(pathname) + 1];
    strcpy(buffer, pathname);    
    char *copy = &buffer[0];
    int dirinumber = ROOT_INUMBER;
    while (copy != NULL) {
        curr = strsep(&copy, "/");
        if (strlen(curr) == 0) {
            continue;
        }
        struct direntv6 de;
        directory_findname(fs, curr, dirinumber, &de);
        dirinumber = de.d_inumber;
    }
    return dirinumber;
}
