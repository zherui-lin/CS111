/**
 * File: alias.cc
 * --------------
 * Simple program in place to illustrate all of the
 * various data types exported by vm.hh
 */

#include <cstdio>
#include <cstring>
#include <exception>
#include "vm.hh"

int main(int argc, char *argv[]) {
    VMRegion vmem(2 * page_size, /* ignored */ nullptr);
    VPage vp1 = vmem.get_base();
    VPage vp2 = vp1 + page_size; // page_size is constant: 4096
    
    PhysMem pm(4);
    PPage pp1 = pm.page_alloc();
    PPage unused = pm.page_alloc();
    PPage pp2 = pm.page_alloc();
    pm.page_free(unused);
    
    // intentionally associate both virtual pages
    // page faults impossible on read or write to vp1 or vp2
    vmem.map(vp1, pp1, PROT_READ | PROT_WRITE);
    vmem.map(vp2, pp2, PROT_READ | PROT_WRITE);
    
    strcpy(vp1, "Copied this into page 1.");
    strcpy(vp2, "Copied this into page 2.");    
    printf("Page 1 says: %s\n", vp1);
    printf("Page 2 says: %s\n", vp2);

    vmem.map(vp1, pp2, PROT_READ);
    vmem.map(vp2, pp1, PROT_READ);
    printf("Page 1 says: %s\n", vp1);
    printf("Page 2 says: %s\n", vp2);

    vmem.map(vp1, pp1, PROT_READ);
    printf("Page 1 says: %s\n", vp1);
    printf("Page 2 says: %s\n", vp2);

    vmem.unmap(vp1);
    vmem.unmap(vp2);
    pm.page_free(pp1);
    pm.page_free(pp2);
    return 0;
}
