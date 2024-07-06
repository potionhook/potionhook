#pragma once
#include <functional>
#include <cstdio>
#include <cstring>
#include "core/logging.hpp"
#include <sys/mman.h>

class BytePatch
{
    void *addr{ nullptr };
    size_t size;
    std::vector<unsigned char> patch_bytes;
    std::vector<unsigned char> original;
    bool patched{ false };

public:
    ~BytePatch()
    {
        Shutdown();
    }

    BytePatch(const std::function<uintptr_t(const char *)> &SigScanFunc, const char *pattern, size_t offset, const std::vector<unsigned char> &patch) : patch_bytes{ patch }
    {
        addr = reinterpret_cast<void *>(SigScanFunc(pattern));
        if (!addr)
        {
            logging::Info("Signature not found");
            throw std::runtime_error("Signature not found");
        }
        addr = static_cast<void *>(static_cast<char *>(addr) + offset);
        size = patch.size();
        original.resize(size);
        Copy();
    }

    BytePatch(uintptr_t addr, const std::vector<unsigned char> &patch) : addr{ reinterpret_cast<void *>(addr) }, patch_bytes{ patch }
    {
        size = patch.size();
        original.resize(size);
        Copy();
    }

    BytePatch(void *addr, const std::vector<unsigned char> &patch) : addr{ addr }, patch_bytes{ patch }
    {
        size = patch.size();
        original.resize(size);
        Copy();
    }

    static void mprotectAddr(unsigned int addr, int size, int flags)
    {
        void *page          = reinterpret_cast<void *>(static_cast<uint64_t>(addr) & ~0xFFF);
        void *end_page      = reinterpret_cast<void *>(static_cast<uint64_t>(addr) + size & ~0xFFF);
        uintptr_t mprot_len = reinterpret_cast<uint64_t>(end_page) - reinterpret_cast<uint64_t>(page) + 0xFFF;

        mprotect(page, mprot_len, flags);
    }

    void Copy()
    {
        void *page          = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) & ~0xFFF);
        void *end_page      = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) + size & ~0xFFF);
        uintptr_t mprot_len = reinterpret_cast<uint64_t>(end_page) - reinterpret_cast<uint64_t>(page) + 0xFFF;

        mprotect(page, mprot_len, PROT_READ | PROT_WRITE | PROT_EXEC);
        memcpy(&original[0], addr, size);
        mprotect(page, mprot_len, PROT_EXEC);
    }

    void Patch()
    {
        if (!patched)
        {
            void *page          = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) & ~0xFFF);
            void *end_page      = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) + size & ~0xFFF);
            uintptr_t mprot_len = reinterpret_cast<uint64_t>(end_page) - reinterpret_cast<uint64_t>(page) + 0xFFF;

            mprotect(page, mprot_len, PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(addr, &patch_bytes[0], size);
            mprotect(page, mprot_len, PROT_EXEC);
            patched = true;
        }
    }

    void Shutdown()
    {
        if (patched)
        {
            void *page          = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) & ~0xFFF);
            void *end_page      = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(addr) + size & ~0xFFF);
            uintptr_t mprot_len = reinterpret_cast<uint64_t>(end_page) - reinterpret_cast<uint64_t>(page) + 0xFFF;

            mprotect(page, mprot_len, PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(addr, &original[0], size);
            mprotect(page, mprot_len, PROT_EXEC);
            patched = false;
        }
    }
};
