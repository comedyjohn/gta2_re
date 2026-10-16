// The mission script interpreter (miss2_0x11C), run on hand-assembled bytecode. Characterisation of the
// control flow opcodes and the counter arithmetic as ported; opcodes that touch the world (cars, peds,
// objects) are covered by the replay corpus, whose missions run them.
#include <doctest.h>

#include <cstring>

#include "Globals.hpp"
#include "ScriptManager_C1EA8.hpp"
#include "ScriptStackFramePool_25C.hpp"
#include "miss2_0x11C.hpp"
#include "miss2_8.hpp"

namespace
{
// Commands live in the script manager's data block; base_pointers[i] is the byte offset of command i
// (0 means "none"), and commands link to each other by that index.
class Script
{
  public:
    Script()
    {
        if (!gScriptStackFramePool_6F8068)
        {
            gScriptStackFramePool_6F8068 = new ScriptStackFramePool_25C();
        }
        mManager = new ScriptManager_C1EA8();
        gScriptManager_6F8060 = mManager;
        mThread = new miss2_0x11C();
    }

    ~Script()
    {
        delete mThread;
        gScriptManager_6F8060 = nullptr;
        delete mManager;
    }

    Script(const Script&) = delete;
    Script& operator=(const Script&) = delete;

    // Command `idx` of the given type, running on to `next` (0xFFFF ends the thread).
    template<class T = SCR_CMD_HEADER>
    T* Add(u16 idx, u16 type, u16 next)
    {
        const u16 offset = static_cast<u16>(idx * 16);
        mManager->field_46C_base_pointers[idx] = offset;
        auto* cmd = reinterpret_cast<T*>(&mManager->field_334C_script_data[offset]);
        cmd->field_0_cmd_this = static_cast<s16>(idx);
        cmd->field_2_type = type;
        cmd->field_4_cmd_next = static_cast<s16>(next);
        return cmd;
    }

    s32& Counter(u16 idx)
    {
        if (!mManager->field_46C_base_pointers[idx])
        {
            Add<SCR_POINTER>(idx, 0, 0xFFFF);
        }
        return reinterpret_cast<SCR_POINTER*>(mManager->GetBasePointer_512770(idx))->field_8_counter;
    }

    // Runs commands until the thread ends or `maxSteps` commands ran; returns the commands run.
    int Run(u16 start, int maxSteps = 100)
    {
        mThread->InitThread_511930(0, start);
        int steps = 0;
        while (mThread->field_4_next_cmd != 0xFFFF && steps < maxSteps)
        {
            mThread->PreExecOpCode_5108D0();
            steps++;
        }
        return steps;
    }

    ScriptManager_C1EA8* mManager;
    miss2_0x11C* mThread;
};
} // namespace

TEST_CASE("a thread runs its commands in order and ends at the terminator")
{
    Script s;
    s.Add(1, SCRCMD_START_EXEC, 2);
    s.Add(2, SCRCMD_START_EXEC, 0xFFFF);
    CHECK(s.Run(1) == 2);
    CHECK(s.mThread->field_4_next_cmd == 0xFFFF);
    CHECK(s.mThread->field_12_exec_flag == 1); // START_EXEC raises the flag once, not twice
}

TEST_CASE("LEVELEND ends the thread")
{
    Script s;
    s.Add(1, SCRCMD_LEVELEND, 2);
    s.Add(2, SCRCMD_START_EXEC, 0xFFFF);
    CHECK(s.Run(1) == 1);
    CHECK(s.mThread->field_12_exec_flag == 0);
}

TEST_CASE("INCREMENT and DECREMENT change a counter")
{
    Script s;
    s32& counter = s.Counter(10);
    counter = 5;
    s.Add<SCR_POINTER>(1, SCRCMD_INCREMENT, 2)->field_8_index = 10;
    s.Add<SCR_POINTER>(2, SCRCMD_INCREMENT, 3)->field_8_index = 10;
    s.Add<SCR_POINTER>(3, SCRCMD_DECREMENT, 0xFFFF)->field_8_index = 10;
    s.Run(1);
    CHECK(counter == 6);
}

TEST_CASE("comparing a counter with a constant sets the condition")
{
    struct Case
    {
        u16 op;
        s32 counter;
        s16 value;
        bool expected;
    };
    const Case cases[] = {
        {SCRCMD_S_LESS_I, 3, 5, true},
        {SCRCMD_S_LESS_I, 5, 5, false},
        {SCRCMD_S_LEQUAL_I, 5, 5, true},
        {SCRCMD_S_GREATER_I, 6, 5, true},
        {SCRCMD_S_GREATER_I, 5, 5, false},
        {SCRCMD_S_GEQUAL_I, 5, 5, true},
        {SCRCMD_S_GEQUAL_I, 4, 5, false},
        {SCRCMD_S_EQUAL_I, 5, 5, true},
        {SCRCMD_S_EQUAL_I, 4, 5, false},
    };
    for (const Case& c : cases)
    {
        CAPTURE(c.op);
        CAPTURE(c.counter);
        Script s;
        s.Counter(10) = c.counter;
        auto* cmd = s.Add<SCR_OPERATE_COUNTER_AND_INT>(1, c.op, 0xFFFF);
        cmd->field_8_counter_idx = 10;
        cmd->field_A_value = c.value;
        s.Run(1);
        CHECK((s.mThread->field_8_cond_result != 0) == c.expected);
    }
}

TEST_CASE("comparing two counters")
{
    struct Case
    {
        u16 op;
        s32 left;
        s32 right;
        bool expected;
    };
    // In the original (this matches 0x506ED0), the four ordering opcodes on two counters compare
    // with the operands swapped: GetOperationType returns the numbers S_OP_S uses for the opposite
    // comparison (LESS_S evaluates "left > right", LEQUAL_S "left >= right", and the reverse for
    // GREATER_S and GEQUAL_S). Reproduced on purpose.
    const Case cases[] = {
        {SCRCMD_S_LESS_S, 3, 5, false},
        {SCRCMD_S_LESS_S, 5, 3, true},
        {SCRCMD_S_GREATER_S, 3, 5, true},
        {SCRCMD_S_GREATER_S, 5, 3, false},
        {SCRCMD_S_LEQUAL_S, 5, 5, true},
        {SCRCMD_S_LEQUAL_S, 4, 5, false},
        {SCRCMD_S_LEQUAL_S, 6, 5, true},
        {SCRCMD_S_GEQUAL_S, 5, 5, true},
        {SCRCMD_S_GEQUAL_S, 6, 5, false},
        {SCRCMD_S_GEQUAL_S, 4, 5, true},
        {SCRCMD_S_EQUAL_S, 7, 7, true},
        {SCRCMD_S_EQUAL_S, 7, 8, false},
    };
    for (const Case& c : cases)
    {
        CAPTURE(c.op);
        Script s;
        s.Counter(10) = c.left;
        s.Counter(11) = c.right;
        auto* cmd = s.Add<SCR_OPERATE_COUNTER_AND_COUNTER_2>(1, c.op, 0xFFFF);
        cmd->field_8_left_counter_idx = 10;
        cmd->field_A_right_counter_idx = 11;
        s.Run(1);
        CHECK((s.mThread->field_8_cond_result != 0) == c.expected);
    }
}

TEST_CASE("IF_JUMP jumps for an AND with a false condition and for an OR with a true one")
{
    struct Case
    {
        u8 isOr;
        s32 counter; // compared with `< 5`
        bool jumps;
    };
    const Case cases[] = {{0, 9, true}, {0, 1, false}, {1, 1, true}, {1, 9, false}};
    for (const Case& c : cases)
    {
        CAPTURE(int(c.isOr));
        CAPTURE(c.counter);
        Script s;
        s.Counter(10) = c.counter;
        s.Counter(11) = 0; // counts how often the "then" part ran
        auto* cmp = s.Add<SCR_OPERATE_COUNTER_AND_INT>(1, SCRCMD_S_LESS_I, 2);
        cmp->field_8_counter_idx = 10;
        cmp->field_A_value = 5;
        auto* jump = s.Add<SCR_IF_JUMP>(2, SCRCMD_IF_JUMP, 3);
        jump->is_or = c.isOr;
        jump->else_endif_pointer = 4;
        s.Add<SCR_POINTER>(3, SCRCMD_INCREMENT, 4)->field_8_index = 11; // "then"
        s.Add(4, SCRCMD_LEVELEND, 0xFFFF);
        s.Run(1);
        CHECK((s.Counter(11) == 0) == c.jumps);
    }
}

TEST_CASE("NOT inverts the condition")
{
    Script s;
    s.Counter(10) = 1;
    auto* cmp = s.Add<SCR_OPERATE_COUNTER_AND_INT>(1, SCRCMD_S_EQUAL_I, 2);
    cmp->field_8_counter_idx = 10;
    cmp->field_A_value = 1;
    s.Add(2, SCRCMD_NOT, 0xFFFF);
    s.Run(1);
    CHECK(s.mThread->field_8_cond_result == 0);
}

TEST_CASE("GOSUB jumps and RETURN comes back, restoring the condition")
{
    Script s;
    s.Counter(10) = 0;
    // 1: GOSUB 5 (next 2)   2: INCREMENT c10 (next 3)   3: LEVELEND
    // 5: INCREMENT c10 (next 6)   6: RETURN
    auto* gosub = s.Add<SCR_TWO_PARAMS>(1, SCRCMD_GOSUB, 2);
    gosub->field_8_unsigned_1 = 5;
    s.Add<SCR_POINTER>(2, SCRCMD_INCREMENT, 3)->field_8_index = 10;
    s.Add(3, SCRCMD_LEVELEND, 0xFFFF);
    s.Add<SCR_POINTER>(5, SCRCMD_INCREMENT, 6)->field_8_index = 10;
    s.Add(6, SCRCMD_RETURN, 0xFFFF);
    s.mThread->InitThread_511930(0, 1);
    s.mThread->field_8_cond_result = true;
    s.mThread->PreExecOpCode_5108D0(); // GOSUB
    CHECK(s.mThread->field_4_next_cmd == 5);
    CHECK(s.mThread->field_114_gosub_stack->field_4_count == 1);
    s.mThread->field_8_cond_result = false;
    s.mThread->PreExecOpCode_5108D0(); // INCREMENT
    s.mThread->PreExecOpCode_5108D0(); // RETURN
    CHECK(s.mThread->field_4_next_cmd == 2);
    CHECK(s.mThread->field_8_cond_result != 0);
    CHECK(s.mThread->field_114_gosub_stack->field_4_count == 0);
    s.mThread->PreExecOpCode_5108D0();
    s.mThread->PreExecOpCode_5108D0();
    CHECK(s.mThread->field_4_next_cmd == 0xFFFF);
    CHECK(reinterpret_cast<SCR_POINTER*>(s.mManager->GetBasePointer_512770(10))->field_8_counter == 2);
}

TEST_CASE("RETURN with nothing to return to ends the thread")
{
    Script s;
    s.Add(1, SCRCMD_RETURN, 0xFFFF);
    CHECK(s.Run(1) == 1);
    CHECK(s.mThread->field_4_next_cmd == 0xFFFF);
}

TEST_CASE("FindCommandByType scans the base pointers from an offset")
{
    Script s;
    s.Add(3, SCRCMD_LEVELSTART, 0xFFFF);
    s.Add(7, SCRCMD_LEVELSTART, 0xFFFF);
    CHECK(s.mManager->FindCommandByType_512100(SCRCMD_LEVELSTART, 0)->field_0_cmd_this == 3);
    CHECK(s.mManager->FindCommandByType_512100(SCRCMD_LEVELSTART, 4)->field_0_cmd_this == 7);
    CHECK(s.mManager->FindCommandByType_512100(SCRCMD_LEVELEND, 0) == nullptr);
}
