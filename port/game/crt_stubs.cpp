#include "crt_stubs.hpp"
#include "Function.hpp"

namespace crt
{
STUB_FUNC(0x5ED3BB)
void* __cdecl malloc(size_t Size)
{
    return ::malloc(Size);
}

STUB_FUNC(0x5ED478)
void __cdecl free(void* Block)
{
    ::free(Block);
}

STUB_FUNC(0x5EE28A);
int __cdecl fseek(FILE* a1, long a2, int a3)
{
    return ::fseek(a1, a2, a3);
}

STUB_FUNC(0x5EE316);
long __cdecl ftell(FILE* f)
{
    return ::ftell(f);
}

STUB_FUNC(0x5EE46E);
int __cdecl fclose(FILE* f)
{
    return ::fclose(f);
}

STUB_FUNC(0x5EE4E4);
FILE* __cdecl fopen(const char* FileName, const char* Mode)
{
    return ::fopen(FileName, Mode);
}

STUB_FUNC(0x5EE4F7);
size_t __cdecl fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fread(Buffer, ElementSize, ElementCount, Stream);
}

STUB_FUNC(0x5EE5DF);
EXPORT size_t __cdecl fwrite(const void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fwrite(Buffer, ElementSize, ElementCount, Stream);
}

STUB_FUNC(0x5EE6E9);
int __cdecl fgetc(FILE* f)
{
    return ::fgetc(f);
}

} // namespace crt

// Destroys count objects of size bytes at ptr, last to first (like the CRT's `??_M`).
void __stdcall __ehvec_dtor(void* ptr, unsigned size, int count, EhVecDtorFn pDtor)
{
    char* p = static_cast<char*>(ptr) + size * count;
    while (--count >= 0)
    {
        p -= size;
        pDtor(p);
    }
}
