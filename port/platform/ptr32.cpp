// The handles behind PTR32 (Source/Ptr32.hpp) in the 64-bit build: pointers the game keeps in 4-byte
// slots of its mission script. A pointer gets a handle the first time it is stored and keeps it, so
// the table only grows with the number of distinct objects a script ever points at. Those come from
// the game's object pools (cars, peds, threads, ...), which keeps it small.
#include "Ptr32.hpp"

#if defined(GTA2_PORT_64BIT)

    #include <stdint.h>

    #include <unordered_map>
    #include <vector>

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
