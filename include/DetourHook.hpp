/*
 * Created by BenCat07 on
 * 11th of May 2020
 *
 */
#pragma once

#include <cstdint>
#include <memory>

#include "bytepatch.hpp"

#define foffset(p, i) ((uint8_t *) &p)[i]

class DetourHook
{
    uintptr_t original_address{};
    std::unique_ptr<BytePatch> patch;
    void *hook_fn{};

public:
    DetourHook() = default;

    DetourHook(uintptr_t original, void *hook_func)
    {
        Init(original, hook_func);
    }

    void Init(uintptr_t original, void *hook_func)
    {
        original_address   = original;
        hook_fn            = hook_func;
        uintptr_t rel_addr = reinterpret_cast<uintptr_t>(hook_func) - original_address - 5;
        patch = std::make_unique<BytePatch>(original, std::vector<uint8_t> { 0xE9, foffset(rel_addr, 0), foffset(rel_addr, 1), foffset(rel_addr, 2), foffset(rel_addr, 3) });
        InitBytepatch();
    }

    void InitBytepatch()
    {
        if (patch)
            patch->Patch();
    }

    void Shutdown()
    {
        if (patch)
            patch->Shutdown();
    }

    ~DetourHook()
    {
        Shutdown();
    }

    // Gets the original function with no patches
    void *GetOriginalFunc() const
    {
        if (patch)
        {
            // Unpatch
            patch->Shutdown();
            return reinterpret_cast<void *>(original_address);
        }
        // No patch, no func.
        return nullptr;
    }

    // Restore the patches
    inline void RestorePatch()
    {
        InitBytepatch();
    }
};