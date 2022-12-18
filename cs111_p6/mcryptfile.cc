
#include <cstring>
#include <iostream>
#include <unordered_map>

#include <sys/stat.h>

#include "mcryptfile.hh"
#include "vm.hh"

std::unique_ptr<PhysMem> MCryptFile::pm = nullptr;
std::size_t MCryptFile::pm_pages = 1000;
std::unordered_map<VPage, MCryptFile::PageInfo*> MCryptFile::page_map;
std::vector<VPage> MCryptFile::page_frames;
std::size_t MCryptFile::next = 0;

struct MCryptFile::PageInfo {
    VPage va;
    PPage pa;
    Prot prot;
    bool dirty;
    class MCryptFile *file;

    PageInfo(VPage va, PPage pa, MCryptFile *file) 
        : va(va), pa(pa), prot(PROT_READ), dirty(false), file(file) {
    }
};

MCryptFile::MCryptFile(Key key, std::string path)
    : CryptFile(key, path)
{
    // You need to implement this
    vmem = nullptr;
}

MCryptFile::~MCryptFile()
{
    MCryptFile::unmap();
}

char *
MCryptFile::map(size_t min_size)
{
    // You need to implement this
    if (pm == nullptr) {
        pm = std::make_unique<PhysMem>(pm_pages);
        page_frames.resize(pm_pages); // have to initialize the size of the vector
    }
    if (vmem == nullptr) {
        vmem = std::make_unique<VMRegion>(std::max(min_size, this->file_size()), [this] (VPage va) { fault(va); });
    }
    return vmem->get_base();
}

void 
MCryptFile::fault(char *va) {
    
    VPage vp = va - std::uintptr_t(va) % page_size;
    if (page_map.find(vp) == page_map.end()) {
        if (pm->nfree() == 0) {
            clock_evict();
        }
        assert(pm->nfree() > 0);
        PPage pp = pm->page_alloc();
        page_map[vp] = new PageInfo(vp, pp, this);
        vmem->map(vp, page_map[vp]->pa, page_map[vp]->prot);
        assert(page_frames[next] == nullptr);
        page_frames[next] = vp; // we have a fixed-size vecotr since we have resized the vector
        next = (next + 1) % page_frames.size(); // move next forward limited by the fixed vector size
        std::size_t offset = vp - vmem->get_base();
        this->aligned_pread(page_map[vp]->pa, page_size, offset);
    } else if (!page_map[vp]->prot & PROT_READ) {
        page_map[vp]->prot = PROT_READ;
        vmem->map(vp, page_map[vp]->pa, page_map[vp]->prot);
    } else {
        page_map[vp]->prot |= PROT_WRITE;
        page_map[vp]->dirty = true;
        vmem->map(vp, page_map[vp]->pa, page_map[vp]->prot);
    }
}

void
MCryptFile::clock_evict() {
    while (true) {
        VPage curr = page_frames[next];
        if (page_map[curr]->prot & PROT_READ) {
            page_map[curr]->prot = PROT_NONE;
            vmem->map(curr, page_map[curr]->pa, page_map[curr]->prot);
            next = (next + 1) % page_frames.size();
        } else {
            if (page_map[curr]->dirty) {
                size_t offset = curr - page_map[curr]->file->vmem->get_base();
                page_map[curr]->dirty = false;
                page_map[curr]->file->aligned_pwrite(page_map[curr]->pa, page_size, offset); // write to the file associated with the page
            }
            vmem->unmap(curr);
            pm->page_free(page_map[curr]->pa);
            delete page_map[curr];
            page_map.erase(curr);
            page_frames[next] = nullptr;
            break;
        }
    }
}

void
MCryptFile::unmap()
{
    // You need to implement this
    flush();
    for (auto iter = page_map.begin(); iter != page_map.end(); ) {
        auto& [key, value] = *iter;
        if (value->file == this) {
            vmem->unmap(key);
            pm->page_free(value->pa);
            delete value;
            iter = page_map.erase(iter);
        } else {
            ++iter;
        }
    }
    vmem = nullptr;
}


void
MCryptFile::flush()
{
    // You need to implement this
    for (auto& [key, value] : page_map) {
        if (value->file == this) {
            if (value->dirty) {
                size_t offset = key - vmem->get_base();
                value->dirty = false;
                aligned_pwrite(value->pa, page_size, offset);
            }
            if (value->prot & PROT_WRITE) {
                value->prot = PROT_READ;
                vmem->map(key, value->pa, value->prot);
            }
        }
    }
}

void
MCryptFile::set_memory_size(std::size_t npages)
{
    // You need to implement this
    pm_pages = npages;
}
