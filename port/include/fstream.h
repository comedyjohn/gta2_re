// The pre-standard <fstream.h> the game was written against, reduced to what ErrorLog (error.hpp)
// uses. Under GCC the game takes its VC6 branches (an undefined _MSC_VER is 0 in #if), which hold a
// classic ofstream by value, 0x3C bytes, followed by field_3C_pLen. This one keeps that size and
// writes through a FILE*.
#pragma once
#include <stdio.h>

class ostream;
typedef ostream& (*ostream_manip)(ostream&);

class ios
{
  public:
    enum open_mode
    {
        in = 0x01,
        out = 0x02,
        ate = 0x04,
        app = 0x08,
        trunc = 0x10,
        nocreate = 0x20,
        noreplace = 0x40,
        binary = 0x80
    };
};

class filebuf
{
  public:
    static const int openprot = 0644;
};

class ostream
{
  public:
    ostream() : mFile(0)
    {
    }

    ostream& operator<<(const char* s)
    {
        if (mFile && s)
        {
            fputs(s, mFile);
        }
        return *this;
    }

    ostream& operator<<(char c)
    {
        if (mFile)
        {
            fputc(c, mFile);
        }
        return *this;
    }

    ostream& operator<<(int v)
    {
        if (mFile)
        {
            fprintf(mFile, "%d", v);
        }
        return *this;
    }

    ostream& operator<<(ostream_manip manip)
    {
        return manip(*this);
    }

    ostream& flush()
    {
        if (mFile)
        {
            fflush(mFile);
        }
        return *this;
    }

  protected:
    FILE* mFile;
};

inline ostream& endl(ostream& os)
{
    return (os << '\n').flush();
}

inline ostream& flush(ostream& os)
{
    return os.flush();
}

class ofstream : public ostream
{
  public:
    ofstream()
    {
    }

    ~ofstream()
    {
        close();
    }

    void open(const char* path, int mode, int /*prot*/ = filebuf::openprot)
    {
        close();
        mFile = fopen(path, (mode & ios::trunc) ? "w" : "a");
    }

    void close()
    {
        if (mFile)
        {
            fclose(mFile);
            mFile = 0;
        }
    }

  private:
    char mPadding[0x3C - sizeof(FILE*)]; // pads to the classic ofstream's size
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(ofstream) == 0x3C, "ErrorLog expects the classic ofstream's size");
#endif
