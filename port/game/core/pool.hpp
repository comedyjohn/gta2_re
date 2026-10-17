#pragma once
#include "core/types.hpp"

// TODO: name these better, this "basic" pool has O(1) allocate
// and deallocate.
template<typename PoolType, s32 PoolSize>
class PoolBasic
{
  public:
    PoolBasic()
    {
        PoolType* iter = &pool_[0];
        for (s32 i = 0; i < PoolSize - 1; i++)
        {
            iter->mpNext = iter + 1;
            iter++;
        }

        pool_[PoolSize - 1].mpNext = nullptr;
        head_ = pool_;
    }

    /*~PoolBasic()
    {
        field_0_pHead = 0;
    }*/

    PoolType* Allocate()
    {
        PoolType* new_item = head_;
        head_ = new_item->mpNext;
        new_item->PoolAllocate();
        return new_item;
    }

    void DeAllocate(PoolType* item)
    {
        item->PoolDeallocate();
        item->mpNext = head_;
        head_ = item;
    }

    PoolType* UnlinkAndReturnNext(PoolType* old_item)
    {
        PoolType* new_item = old_item->mpNext;
        old_item->mpNext = this->head_;
        this->head_ = old_item;
        return new_item;
    }

    PoolType* head_;
    PoolType pool_[PoolSize];
};

template<typename PoolType, s32 PoolSize>
class Pool
{
  public:
    bool HasPStart()
    {
        return start_ != nullptr;
    }

    // 9.6f 0x4B8FD0 (CokeZero_FC, 10.5 copy 0x5935C0): first used item
    PoolType* GetFirstUsed()
    {
        return prev_;
    }

    Pool()
    {
        PoolType* iter = pool_;
        for (s32 i = 0; i < PoolSize - 1; i++)
        {
            iter->mpNext = iter + 1;
            iter++;
        }

        pool_[PoolSize - 1].mpNext = nullptr;
        start_ = pool_;

        prev_ = nullptr;
        field_x_count_ = 0;
    }

    ~Pool()
    {
        start_ = nullptr;
        prev_ = nullptr;
    }

    PoolType* Allocate()
    {
        PoolType* tmp = start_;
        start_ = tmp->mpNext;
        tmp->mpNext = prev_;
        prev_ = tmp;
        tmp->PoolAllocate();
        return tmp;
    }

    // alloc_append_4254A0
    PoolType* AllocateAppend()
    {
        // Pop from free list
        PoolType* obj = start_;
        start_ = obj->mpNext;
        obj->mpNext = nullptr;

        // Append to active list (FIFO style)
        if (!prev_)
        {
            prev_ = obj; // active list was empty
        }
        else
        {
            PoolType* tail = prev_;
            while (tail->mpNext)
            {
                tail = tail->mpNext;
            }
            tail->mpNext = obj;
        }

        obj->PoolAllocate(); // per-object init
        return obj;
    }

    PoolType* AllocateUntracked()
    {
        PoolType* tmp = this->start_;
        this->start_ = tmp->mpNext;
        tmp->mpNext = nullptr;
        tmp->PoolAllocate();
        return tmp;
    }

    void DeAllocate(PoolType* item)
    {
        item->PoolDeallocate();
        item->mpNext = start_;
        start_ = item;
    }

    void RemoveNoDeAllocate(PoolType* item)
    {
        item->mpNext = start_;
        start_ = item;
    }

    void UpdateNextPrev(PoolType* item)
    {
        item->mpNext = prev_;
        prev_ = item;
    }

    void UpdatePool()
    {
        field_x_count_ = 0;

        PoolType* previous_item = nullptr;
        PoolType* curr_item = prev_;

        while (curr_item)
        {
            ++field_x_count_;

            PoolType* next = curr_item->mpNext;

            if (curr_item->PoolUpdate())
            {
                curr_item->PoolDeallocate();

                if (previous_item && previous_item->mpNext != curr_item)
                {
                    previous_item = nullptr;
                }

                if (previous_item)
                {
                    previous_item->mpNext = curr_item->mpNext;
                }
                else
                {
                    if (prev_ == curr_item)
                    {
                        prev_ = curr_item->mpNext;
                    }
                    else
                    {
                        previous_item = prev_;
                        while (previous_item->mpNext != curr_item)
                        {
                            previous_item = previous_item->mpNext;
                        }
                        previous_item->mpNext = curr_item->mpNext;
                    }
                }

                curr_item->mpNext = start_;
                start_ = curr_item;
            }
            else
            {
                previous_item = curr_item;
            }

            curr_item = next;
        }
    }

    void UpdatePoolNoDeallocate()
    {
        field_x_count_ = 0;

        PoolType* previous_item = nullptr;
        PoolType* curr_item = prev_;

        while (curr_item)
        {
            ++field_x_count_;

            PoolType* next = curr_item->mpNext;

            if (curr_item->PoolUpdate())
            {
                if (previous_item && previous_item->mpNext != curr_item)
                {
                    previous_item = nullptr;
                }

                if (previous_item)
                {
                    previous_item->mpNext = curr_item->mpNext;
                }
                else
                {
                    if (prev_ == curr_item)
                    {
                        prev_ = curr_item->mpNext;
                    }
                    else
                    {
                        previous_item = prev_;
                        while (previous_item->mpNext != curr_item)
                        {
                            previous_item = previous_item->mpNext;
                        }
                        previous_item->mpNext = curr_item->mpNext;
                    }
                }

                curr_item->mpNext = start_;
                start_ = curr_item;
            }
            else
            {
                previous_item = curr_item;
            }

            curr_item = next;
        }
    }

    PoolType* FindAndDeAllocate(PoolType* to_find)
    {
        PoolType* last = nullptr;

        for (PoolType* iter = prev_; iter; iter = iter->mpNext)
        {
            if (iter == to_find)
            {
                iter->PoolDeallocate();

                if (last)
                {
                    last->mpNext = iter->mpNext;
                }
                else
                {
                    prev_ = iter->mpNext;
                }
                iter->mpNext = start_;
                start_ = iter;
                return to_find;
            }
            last = iter;
        }
        return to_find;
    }

    void UnlinkFromActiveList(PoolType* to_find)
    {
        PoolType* last = nullptr;
        for (PoolType* iter = this->prev_; iter; iter = iter->mpNext)
        {
            if (iter == to_find)
            {
                if (last)
                {
                    last->mpNext = iter->mpNext;
                }
                else
                {
                    this->prev_ = iter->mpNext;
                }
                iter->mpNext = nullptr;
                return;
            }
            last = iter;
        }
    }

    PoolType* start_;
    PoolType* prev_;
    PoolType pool_[PoolSize];

    // Unknown offsets due to sizeof(PoolType) varying per template
    s16 field_x_count_;
    s16 field_x_pad_;
};
