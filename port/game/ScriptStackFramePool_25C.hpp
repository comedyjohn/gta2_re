#pragma once

#include "Function.hpp"
#include "Pool.hpp"

// A script stack frame (pushed by GOSUB): where to return to and the condition result at the time of the call
class ScriptStackFrame_C
{
  public:
    ScriptStackFrame_C();
    ~ScriptStackFrame_C();
    void PoolAllocate();

    void PoolDeallocate()
    {
    }

    s32 field_0_cond_result;
    s16 field_4_return_cmd;
    ScriptStackFrame_C* mpNext;
};

// Pool of the stack frames (GOSUB return info) of all script threads (miss2_8)
class ScriptStackFramePool_25C
{
  public:
    enum
    {
        k_max_frames = 50
    };

    ScriptStackFramePool_25C() = default;

    ~ScriptStackFramePool_25C();

    // 9.6f 0x476780
    void DeAllocate_476780(ScriptStackFrame_C* pItem)
    {
        field_0_pool.DeAllocate(pItem);
    }

    PoolBasic<ScriptStackFrame_C, k_max_frames> field_0_pool;
};

EXTERN_GLOBAL(ScriptStackFramePool_25C*, gScriptStackFramePool_6F8068);
