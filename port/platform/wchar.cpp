// UTF-16 string functions with MSVC semantics (see port/include/port_wchar.h). wchar_t is 2 bytes in
// this build (-fshort-wchar), so glibc's wcs* functions, which assume 4, are never used.
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern "C" {

size_t port_wcslen(const wchar_t* s)
{
    const wchar_t* p = s;
    while (*p)
    {
        p++;
    }
    return p - s;
}

wchar_t* port_wcscpy(wchar_t* dst, const wchar_t* src)
{
    wchar_t* d = dst;
    while ((*d++ = *src++) != 0)
    {
    }
    return dst;
}

wchar_t* port_wcsncpy(wchar_t* dst, const wchar_t* src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i]; i++)
    {
        dst[i] = src[i];
    }
    for (; i < n; i++)
    {
        dst[i] = 0;
    }
    return dst;
}

int port_wcscmp(const wchar_t* a, const wchar_t* b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return static_cast<int>(static_cast<unsigned short>(*a)) - static_cast<int>(static_cast<unsigned short>(*b));
}

// GTA2's text is Latin-1 inside UTF-16 (code points below 256), and its multibyte strings are the
// Windows-1252 ANSI code page, so the conversions are a byte per code unit.
size_t port_wcstombs(char* dst, const wchar_t* src, size_t n)
{
    size_t len = port_wcslen(src);
    if (!dst)
    {
        return len;
    }
    size_t i = 0;
    for (; i < n && src[i]; i++)
    {
        unsigned short c = static_cast<unsigned short>(src[i]);
        dst[i] = c < 256 ? static_cast<char>(c) : '?';
    }
    if (i < n)
    {
        dst[i] = 0;
    }
    return i;
}

size_t port_mbstowcs(wchar_t* dst, const char* src, size_t n)
{
    size_t len = strlen(src);
    if (!dst)
    {
        return len;
    }
    size_t i = 0;
    for (; i < n && src[i]; i++)
    {
        dst[i] = static_cast<unsigned char>(src[i]);
    }
    if (i < n)
    {
        dst[i] = 0;
    }
    return i;
}

wchar_t* port_itow(int value, wchar_t* buf, int radix)
{
    char tmp[40];
    if (radix == 10)
    {
        snprintf(tmp, sizeof(tmp), "%d", value);
    }
    else
    {
        // MSVC's _itow treats a non-decimal value as unsigned
        unsigned int v = static_cast<unsigned int>(value);
        char* p = tmp + sizeof(tmp) - 1;
        *p = 0;
        do
        {
            unsigned int digit = v % radix;
            *--p = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
            v /= radix;
        } while (v);
        memmove(tmp, p, strlen(p) + 1);
    }
    port_mbstowcs(buf, tmp, strlen(tmp) + 1);
    return buf;
}

// MSVC's swprintf without a size: %s and %c take wide arguments (%hs/%S narrow ones), and the
// numeric conversions behave as printf's.
int port_vswprintf(wchar_t* buf, const wchar_t* fmt, va_list args)
{
    wchar_t* out = buf;
    while (*fmt)
    {
        if (*fmt != L'%')
        {
            *out++ = *fmt++;
            continue;
        }

        // Copy the conversion spec to a narrow format for snprintf
        char spec[32];
        size_t len = 0;
        spec[len++] = '%';
        fmt++;
        if (*fmt == L'%')
        {
            *out++ = L'%';
            fmt++;
            continue;
        }
        while (*fmt && strchr("-+ #0123456789.*", static_cast<char>(*fmt)) && len < sizeof(spec) - 4)
        {
            spec[len++] = static_cast<char>(*fmt++);
        }
        bool narrow = false;
        bool wide = false;
        bool isLong = false;
        bool isLongLong = false;
        while (*fmt == L'h' || *fmt == L'l' || *fmt == L'L' || *fmt == L'I' || *fmt == L'w')
        {
            if (*fmt == L'h')
            {
                narrow = true;
            }
            else if (*fmt == L'w')
            {
                wide = true;
            }
            else if (*fmt == L'l')
            {
                isLongLong = isLong;
                isLong = true;
                wide = true;
            }
            else if (*fmt == L'I' && fmt[1] == L'6' && fmt[2] == L'4')
            {
                isLongLong = true;
                fmt += 2;
            }
            fmt++;
        }
        wchar_t conv = *fmt ? *fmt++ : 0;
        const bool starWidth = memchr(spec, '*', len) != 0;
        int starArg = starWidth ? va_arg(args, int) : 0;

        char tmp[512];
        tmp[0] = 0;
        switch (conv)
        {
            case L's':
            case L'S':
            {
                const bool argIsWide = (conv == L's') ? !narrow : wide;
                spec[len++] = 's';
                spec[len] = 0;
                if (argIsWide)
                {
                    const wchar_t* ws = va_arg(args, const wchar_t*);
                    if (!ws)
                    {
                        ws = L"(null)";
                    }
                    // Width/precision count characters; apply them on a narrowed copy
                    char narrowCopy[512];
                    size_t n = port_wcslen(ws);
                    if (n >= sizeof(narrowCopy))
                    {
                        n = sizeof(narrowCopy) - 1;
                    }
                    for (size_t i = 0; i < n; i++)
                    {
                        // Outside Latin-1 becomes ? here (only when a width or precision is given)
                        narrowCopy[i] = static_cast<unsigned short>(ws[i]) < 256 ? static_cast<char>(ws[i]) : '?';
                    }
                    narrowCopy[n] = 0;
                    if (len == 2 && !starWidth) // plain %s: copy the wide string as it is
                    {
                        port_wcscpy(out, ws);
                        out += port_wcslen(ws);
                        continue;
                    }
                    starWidth ? snprintf(tmp, sizeof(tmp), spec, starArg, narrowCopy) : snprintf(tmp, sizeof(tmp), spec, narrowCopy);
                }
                else
                {
                    const char* s = va_arg(args, const char*);
                    starWidth ? snprintf(tmp, sizeof(tmp), spec, starArg, s ? s : "(null)")
                              : snprintf(tmp, sizeof(tmp), spec, s ? s : "(null)");
                }
                break;
            }
            case L'c':
            case L'C':
            {
                const bool argIsWide = (conv == L'c') ? !narrow : wide;
                int c = va_arg(args, int);
                *out++ = argIsWide ? static_cast<wchar_t>(c) : static_cast<wchar_t>(static_cast<unsigned char>(c));
                continue;
            }
            case L'd':
            case L'i':
            case L'u':
            case L'x':
            case L'X':
            case L'o':
            {
                if (isLongLong)
                {
                    spec[len++] = 'l';
                    spec[len++] = 'l';
                }
                spec[len++] = static_cast<char>(conv);
                spec[len] = 0;
                if (isLongLong)
                {
                    long long v = va_arg(args, long long);
                    starWidth ? snprintf(tmp, sizeof(tmp), spec, starArg, v) : snprintf(tmp, sizeof(tmp), spec, v);
                }
                else
                {
                    int v = va_arg(args, int);
                    starWidth ? snprintf(tmp, sizeof(tmp), spec, starArg, v) : snprintf(tmp, sizeof(tmp), spec, v);
                }
                break;
            }
            case L'f':
            case L'e':
            case L'E':
            case L'g':
            case L'G':
            {
                spec[len++] = static_cast<char>(conv);
                spec[len] = 0;
                double v = va_arg(args, double);
                starWidth ? snprintf(tmp, sizeof(tmp), spec, starArg, v) : snprintf(tmp, sizeof(tmp), spec, v);
                break;
            }
            case L'p':
            {
                spec[len++] = 'p';
                spec[len] = 0;
                void* v = va_arg(args, void*);
                snprintf(tmp, sizeof(tmp), spec, v);
                break;
            }
            default:
                // Unknown conversion: emit it as written
                *out++ = L'%';
                if (conv)
                {
                    *out++ = conv;
                }
                continue;
        }
        for (const char* p = tmp; *p; p++)
        {
            *out++ = static_cast<wchar_t>(static_cast<unsigned char>(*p));
        }
    }
    *out = 0;
    return static_cast<int>(out - buf);
}

int port_swprintf(wchar_t* buf, const wchar_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = port_vswprintf(buf, fmt, args);
    va_end(args);
    return n;
}

} // extern "C"
