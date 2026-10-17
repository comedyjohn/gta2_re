#include "rng.hpp"
#include "Hud.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "debug.hpp"
#include "error.hpp"
#include <cstdio>
#include <cstdlib>

DEFINE_GLOBAL(Rng*, gpRng_67AB34, 0x67AB34);
DEFINE_GLOBAL(Rng, gRng_6F6784, 0x6F6784);

EXTERN_GLOBAL_ARRAY(wchar_t, tmpBuff_67BD9C, 640);

// 0x48B900
void Rng::AdvanceCycle()
{
    ++rng_;
    rnd_ = gRng_6F6784.Rand();
}

// 0x48B920
void Rng::ShowCycle()
{
    if (gInputRecorder_67F8B0.inlined_check())
    {
        swprintf(tmpBuff_67BD9C, L"%d / %d", this->rng_, gInputRecorder_67F8B0.GetLastRecRngIdx_44AA80());
    }
    else
    {
        swprintf(tmpBuff_67BD9C, L"%d", this->rng_);
    }
    gHud_2B00_706620->field_650_texts.DisplayText_5D1F50(tmpBuff_67BD9C, 0, 460, gDebugFont_706600, 1);
}

// 0x4F7A40
void Rng::Srand()
{
    srand(1);
}

// 0x4F7AE0
s16 Rng::GetInt(const s16& max_rnd)
{
    if (bLog_random_67D5FC)
    {
        if (!max_rnd)
        {
            this->rng_ = 0;
        }
        else
        {
            this->rng_ = rand() % max_rnd;
        }
        sprintf(gTmpBuffer_67C598, "%d: random (get_int) %d", gpRng_67AB34->GetCurRng(), static_cast<s16>(this->GetCurRng()));
        gFile_67C530.Write_4D9620(gTmpBuffer_67C598);
        return static_cast<s16>(this->GetCurRng());
    }
    else
    {
        if (!max_rnd)
        {
            return 0;
        }
        else
        {
            return rand() % max_rnd;
        }
    }
}

// 0x4F7B70
u8 Rng::GetUint8(const u8& max_rnd)
{
    if (bLog_random_67D5FC)
    {
        if (!max_rnd)
        {
            this->rng_ = 0;
        }
        else
        {
            this->rng_ = rand() % max_rnd;
        }
        sprintf(gTmpBuffer_67C598, "%d: random (get_uint8) %d", gpRng_67AB34->GetCurRng(), static_cast<u8>(rng_));
        gFile_67C530.Write_4D9620(gTmpBuffer_67C598);
        return static_cast<u8>(this->rng_);
    }
    else
    {
        if (!max_rnd)
        {
            return 0;
        }
        else
        {
            return rand() % max_rnd;
        }
    }
}

// 0x4F7C00
int Rng::Rand()
{
    return rand();
}