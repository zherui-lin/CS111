#include <cstdio>
#include <random>

#include "vm.hh"

struct FaultCatcher {
    PhysMem pm;
    VMRegion vmem;
    VPage last_page = nullptr;
    PPage ppage = pm.page_alloc();
    
    FaultCatcher() : pm(4), vmem(0x10'000, [this](char *va){ fault(va); }) {}
    ~FaultCatcher() {
        if (last_page) vmem.unmap(last_page);
        pm.page_free(ppage);
    }
    
    void fault(char *va) {
        VPage vp = va - std::uintptr_t(va) % page_size;
        printf("\tPage fault at %p (page 0x%lx)\n", va,
               (vp - vmem.get_base())/page_size);
        if (last_page) vmem.unmap(last_page);
        last_page = vp;
        vmem.map(vp, ppage, PROT_READ | PROT_WRITE);
    }
};

int main(int argc, char *argv[]) {
    FaultCatcher fc;
    char *base = fc.vmem.get_base();
    std::default_random_engine dre;    
    std::uniform_int_distribution<int> rnd(0, fc.vmem.get_size() - 2);
    for (size_t i = 1; i <= 1000; ++i) {
        size_t idx = rnd(dre);
        printf("\tAbout to write to offset 0x%lx (%p)\n", idx, &base[idx]);
        base[idx] = 'A';
        printf("\tAbout to write to offset 0x%lx (%p)\n", idx + 1, &base[idx + 1]);
        base[idx + 1] = '+';
        printf("\n");
    }
    return 0;
}
