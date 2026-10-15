#pragma once

#include "Function.hpp"
#include <windows.h>

#include <cstdio>

// A text log file, such as the mission script's test\MISS_LOG.TXT. Writing to a log that was
// never opened does nothing.
class ErrorLog
{
  public:
    ErrorLog() = default;
    ErrorLog(const char* path, int mode); // 0x4D94E0
    ~ErrorLog();                          // 0x4A3610: logs the intro line again, then closes
    ErrorLog(const ErrorLog&) = delete;
    ErrorLog& operator=(const ErrorLog&) = delete;

    // 0x4D9470: mode 1 starts the file over, anything else appends to it
    void Open_4D9470(const char* path, s32 mode);
    void Write_4D9620(const char* pMsg);       // the text and a new line
    void Write_Log_4D9650(const char* buffer); // the text as it is
    void log_timestamp_4D9540();
    void log_intro_4D95A0();

  private:
    void Write(const char* text, bool newLine);

    FILE* mFile = nullptr;
};

void Error_SetName_4A0770(const char_type* pFileName);

void FatalError_4A38C0(s32 Code, const char_type* pSourceFile, s32 lineNo, ...);
void FatalDXError_4A3CF0(HRESULT hr, const char_type* pSourceFile, s32 lineNo);
void Shutdown_4DA740();

void GBH_Graphis_DMA_Video_Free_5D9830();

EXTERN_GLOBAL(char_type, gTmpBuffer_67C598[256]);
EXTERN_GLOBAL(HWND, gHwnd_707F04);
EXTERN_GLOBAL(char_type, bDestroyed_6F5B70);
EXTERN_GLOBAL(char_type, gErrStr_67C29C[256]);
EXTERN_GLOBAL(char_type, gErrStr2_67C3A8[256]);

EXTERN_GLOBAL(ErrorLog, gErrorLog_67C530);
EXTERN_GLOBAL(ErrorLog, gMiss2Log_6F7698);
extern ErrorLog& gFile_67C530; // the same log as gErrorLog_67C530 (both are 0x67C530)
