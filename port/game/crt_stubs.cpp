#include "crt_stubs.hpp"
#include "Function.hpp"

namespace crt
{
// 0x5ED3BB
void* malloc(size_t Size)
{
    return ::malloc(Size);
}

// 0x5ED478
void free(void* Block)
{
    ::free(Block);
}

// 0x5EE28A
int fseek(FILE* a1, long a2, int a3)
{
    return ::fseek(a1, a2, a3);
}

// 0x5EE316
long ftell(FILE* f)
{
    return ::ftell(f);
}

// 0x5EE46E
int fclose(FILE* f)
{
    return ::fclose(f);
}

// 0x5EE4E4
FILE* fopen(const char* FileName, const char* Mode)
{
    return ::fopen(FileName, Mode);
}

// 0x5EE4F7
size_t fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fread(Buffer, ElementSize, ElementCount, Stream);
}

// 0x5EE5DF
size_t fwrite(const void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fwrite(Buffer, ElementSize, ElementCount, Stream);
}

// 0x5EE6E9
int fgetc(FILE* f)
{
    return ::fgetc(f);
}

} // namespace crt
