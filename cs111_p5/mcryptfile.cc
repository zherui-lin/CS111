
#include <cstring>
#include <iostream>
#include <unordered_map>

#include <sys/stat.h>

#include "mcryptfile.hh"
#include "vm.hh"

std::unique_ptr<PhysMem> MCryptFile::pm = nullptr;
std::size_t MCryptFile::pm_pages = 1000;
std::unordered_map<VPage, MCryptFile::PageInfo*> MCryptFile::page_map;

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
    }
    if (vmem == nullptr) {
        vmem = std::make_unique<VMRegion>(std::max(min_size, this->file_size()), [this] (VPage va) { fault(va); });
    }
    return vmem->get_base();
}

void 
MCryptFile::fault(char *va) {
    std::size_t page_size = get_page_size();
    VPage vp = va - std::uintptr_t(va) % page_size;
    if (page_map.find(vp) == page_map.end()) {
        PPage pp = pm->page_alloc();
        vmem->map(vp, pp, PROT_READ);
        page_map[vp] = new PageInfo({pp, PROT_READ, this});
        std::size_t offset = vp - vmem->get_base();
        this->aligned_pread(pp, page_size, offset);
    } else {
        page_map[vp]->prot = PROT_READ | PROT_WRITE;
        vmem->map(vp, page_map[vp]->pa, page_map[vp]->prot);
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
            iter = page_map.erase(iter);
            delete value;
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
    std::size_t page_size = get_page_size();
    for (auto& [key, value] : page_map) {
        if (value->file == this) {
            if (value->prot & PROT_WRITE) {
                size_t offset = key - vmem->get_base();
                value->prot = PROT_READ;
                this->aligned_pwrite(value->pa, page_size, offset);
                vmem->map(key, value->pa, PROT_READ);
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
