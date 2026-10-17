// The handles behind PTR32 (port/game/Ptr32.hpp) in the 64-bit build: pointers the game keeps in 4-byte
// slots of its mission script. A pointer gets a handle the first time it is stored and keeps it, so
// the table only grows with the number of distinct objects a script ever points at. Those come from
// the game's object pools (cars, peds, threads, ...), which keeps it small.
// In both builds it also keeps which slots a pointer was stored in, for the state hash.
#include "core/ptr32.hpp"
#include "statehash.h"

#include <stdint.h>
#include <string.h>

#include <unordered_map>
#include <vector>

namespace
{
// The slots a pointer was stored in, with the handle stored. A slot that holds something else now
// (the script reused the memory) no longer holds that handle.
std::unordered_map<const void*, u32>& Slots()
{
    static std::unordered_map<const void*, u32> slots;
    return slots;
}
} // namespace

void Ptr32_NoteSlot(const void* slot, u32 handle)
{
    Slots()[slot] = handle;
}

void Ptr32_SlotsIn(const void* base, uint32_t len, void (*f)(uint32_t offset, void* ctx), void* ctx)
{
    const char* begin = static_cast<const char*>(base);
    for (const std::pair<const void* const, u32>& slot : Slots())
    {
        const char* p = static_cast<const char*>(slot.first);
        if (p < begin || p + sizeof(u32) > begin + len)
        {
            continue;
        }
        u32 now;
        memcpy(&now, p, sizeof(now));
        if (now == slot.second)
        {
            f(static_cast<uint32_t>(p - begin), ctx);
        }
    }
}

#if defined(GTA2_PORT_64BIT)

namespace
{
std::unordered_map<const void*, u32>& Handles()
{
    static std::unordered_map<const void*, u32> handles;
    return handles;
}

std::vector<void*>& Pointers()
{
    static std::vector<void*> pointers(1, nullptr); // handle 0 is null
    return pointers;
}
} // namespace

u32 Ptr32_ToHandle(const void* p)
{
    if (!p)
    {
        return 0;
    }
    std::unordered_map<const void*, u32>& handles = Handles();
    std::unordered_map<const void*, u32>::const_iterator it = handles.find(p);
    if (it != handles.end())
    {
        return it->second;
    }
    std::vector<void*>& pointers = Pointers();
    const u32 handle = static_cast<u32>(pointers.size());
    pointers.push_back(const_cast<void*>(p));
    handles.emplace(p, handle);
    return handle;
}

void* Ptr32_FromHandle(u32 handle)
{
    std::vector<void*>& pointers = Pointers();
    if (handle >= pointers.size())
    {
        // Script data that never held a stored pointer, read as one (a command whose slot holds an
        // index): give back the value as the 32-bit game would have seen it
        return reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    }
    return pointers[handle];
}

#endif
