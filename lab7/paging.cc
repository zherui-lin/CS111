#include <cassert>
#include <cstdio>
#include <new>
#include <memory>
#include "vm.hh"

/* forward declaration */
struct PageTableEntry;
static std::unique_ptr<PhysMem> pm;
static std::unordered_map<VPage, struct PageTableEntry *> page_table;

static std::vector<VPage> page_frames;
static std::size_t next = 0;

/**
 * Record: PageTableEntry
 * ----------------------
 * Supplemental page table record for system-wide
 * page table.
 */
struct PageTableEntry {
    VPage vp;
    PPage pp = nullptr;
    Prot prot;
    bool accessed = false, dirty = false;
    
    PageTableEntry(VPage vp, Prot p)
        : vp(vp) {
        if (pm->nfree() == 0) {
            printf("Out of pages... evicting round robin.\n");
            evict_next();
            assert(pm->nfree() > 0);
        }
        pp = pm->page_alloc();
        protect(p);
        assert(page_frames[next] == nullptr);
        page_frames[next] = vp;
        next = (next + 1) % page_frames.size();
    }
    
    ~PageTableEntry() {
        VMRegion::unmap(vp);
        pm->page_free(pp);
    }
    
    void protect(Prot p) {
        prot = p;
        accessed = prot & PROT_READ;
        dirty = prot & PROT_WRITE;
        VMRegion::map(vp, pp, prot);
    }

    void evict_next() {
        delete page_table[page_frames[next]];
        page_table.erase(page_frames[next]);
        page_frames[next] = nullptr;
    }
};

struct TrackRegion {
    VMRegion vmem;
    
    TrackRegion(std::size_t nbytes)
        : vmem(nbytes, [this](char *a){ handler(a); }) {}
    ~TrackRegion();    
    void handler(char *addr);
    char &operator[](std::ptrdiff_t i) { 
        assert(i >= 0 && std::size_t(i) < vmem.get_size());
        return vmem.get_base()[i]; 
    }
};

TrackRegion::~TrackRegion() {
    for (auto iter = page_table.begin(); iter != page_table.end(); ++iter) {
        delete iter->second;
    }
    page_table.clear();
}

static std::string prot_str(Prot p) {
    std::string str("---");
    str[0] = p & PROT_READ ? 'R' : '-';
    str[1] = p & PROT_WRITE ? 'W' : '-';
    str[2] = p & PROT_EXEC ? 'X' : '-';
    return str;
}

void TrackRegion::handler(char *addr) {
    VPage vp = addr - std::intptr_t(addr) % page_size;
    PageTableEntry *entry = page_table[vp];
    if (entry == nullptr) entry = page_table[vp] = new PageTableEntry(vp, PROT_NONE);
    Prot prot = PROT_READ;
    if (entry->accessed || entry->dirty) prot |= PROT_WRITE;
    entry->protect(prot);
    printf("Just mapped %p with prot %s\n", vp, prot_str(prot).c_str());
}

static void dump_pt() {
    for (const std::pair<VPage, PageTableEntry *>& p: page_table) {
        printf("page %p prot %s is %s\n", p.first,
               prot_str(p.second->prot).c_str(),
               p.second->dirty ? "dirty" : p.second->accessed ? "accessed" : "untouched");
    }
}

static size_t extract_value(int argc, char *argv[], int pos, int dflt) {
    if (pos >= argc) return dflt;
    if (atoi(argv[pos]) == 0) return dflt;
    return size_t(atoi(argv[pos]));
}

int main(int argc, char *argv[]) {
    size_t vpgs = extract_value(argc, argv, 1, 12),
           ppgs = extract_value(argc, argv, 2, 8);
    
    pm.reset(new PhysMem(ppgs));
    page_frames.resize(ppgs); /* have to determine the size of the page vector!! */
    TrackRegion tr(vpgs * page_size);
    for (size_t i = 0; i < vpgs; ++i) {
        char c;
        switch (i % 3) {
        case 0: c = tr[i * page_size]; break;
        case 1: tr[i * page_size] = c; break;
        case 2: /* do nothing!! */ break;
        }
    }
    printf("==== dumping auxiliary page table ====\n");
    dump_pt();
    return 0;
}
