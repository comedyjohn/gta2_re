#pragma once
#include "Function.hpp"
#include <stdio.h>
#include <stdlib.h>

// crt proxy functions - required so that the hook build has fopen/fread/any crt function
// that we call using the same "copy" of the crt. I.e the one in the executable, not the one
// linked into the hook dll, else ka boom m8.
// Mixing VC6+ CRT structs with VC65CRT structs will likely crash also, howwever if all CRT funcs are matched or stubbed
// they'll use the same copy of the structure and it will be fine...

namespace crt
{
long ftell(FILE*);
int fseek(FILE*, long, int);
FILE* fopen(const char* FileName, const char* Mode);
int fclose(FILE*);
size_t fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);
int fgetc(FILE*);
size_t fwrite(const void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);

void free(void* Block);
void* malloc(size_t Size);

// TODO: crt::malloc, crt::free

} // namespace crt

// VC6 maps this name to the CRT's vector destructor iterator `??_M`, which compiler generated
// static array destructors call. The CRT's version takes a destructor; VC6 has no
// keyword, so this one (`??_M@YGXPAXIHP6AX0@Z@Z`, defined in crt_stubs.cpp) takes a
// destructor thunk instead.
using EhVecDtorFn = void (*)(void*);
void EhVecDtor(void* ptr, unsigned size, int count, EhVecDtorFn pDtor);
