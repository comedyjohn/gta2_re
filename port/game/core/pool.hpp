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
            iter->next_ = iter + 1;
            iter++;
        }

        pool_[PoolSize - 1].next_ = nullptr;
        head_ = pool_;
    }

    /*~PoolBasic()
    {
        field_0_pHead = 0;
    }*/

    PoolType* Allocate()
    {
        PoolType* new_item = head_;
        head_ = new_item->next_;
        new_item->PoolAllocate();
        return new_item;
    }

    void DeAllocate(PoolType* item)
    {
        item->PoolDeallocate();
        item->next_ = head_;
        head_ = item;
    }

    PoolType* UnlinkAndReturnNext(PoolType* old_item)
    {
        PoolType* new_item = old_item->next_;
        old_item->next_ = this->head_;
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
            iter->next_ = iter + 1;
            iter++;
        }

        pool_[PoolSize - 1].next_ = nullptr;
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
        start_ = tmp->next_;
        tmp->next_ = prev_;
        prev_ = tmp;
        tmp->PoolAllocate();
        return tmp;
    }

    // alloc_append_4254A0
    PoolType* AllocateAppend()
    {
        // Pop from free list
        PoolType* obj = start_;
        start_ = obj->next_;
        obj->next_ = nullptr;

        // Append to active list (FIFO style)
        if (!prev_)
        {
            prev_ = obj; // active list was empty
        }
        else
        {
            PoolType* tail = prev_;
            while (tail->next_)
            {
                tail = tail->next_;
            }
            tail->next_ = obj;
        }

        obj->PoolAllocate(); // per-object init
        return obj;
    }

    PoolType* AllocateUntracked()
    {
        PoolType* tmp = this->start_;
        this->start_ = tmp->next_;
        tmp->next_ = nullptr;
        tmp->PoolAllocate();
        return tmp;
    }

    void DeAllocate(PoolType* item)
    {
        item->PoolDeallocate();
        item->next_ = start_;
        start_ = item;
    }

    void RemoveNoDeAllocate(PoolType* item)
    {
        item->next_ = start_;
        start_ = item;
    }

    void UpdateNextPrev(PoolType* item)
    {
        item->next_ = prev_;
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

            PoolType* next = curr_item->next_;

            if (curr_item->PoolUpdate())
            {
                curr_item->PoolDeallocate();

                if (previous_item && previous_item->next_ != curr_item)
                {
                    previous_item = nullptr;
                }

                if (previous_item)
                {
                    previous_item->next_ = curr_item->next_;
                }
                else
                {
                    if (prev_ == curr_item)
                    {
                        prev_ = curr_item->next_;
                    }
                    else
                    {
                        previous_item = prev_;
                        while (previous_item->next_ != curr_item)
                        {
                            previous_item = previous_item->next_;
                        }
                        previous_item->next_ = curr_item->next_;
                    }
                }

                curr_item->next_ = start_;
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

            PoolType* next = curr_item->next_;

            if (curr_item->PoolUpdate())
            {
                if (previous_item && previous_item->next_ != curr_item)
                {
                    previous_item = nullptr;
                }

                if (previous_item)
                {
                    previous_item->next_ = curr_item->next_;
                }
                else
                {
                    if (prev_ == curr_item)
                    {
                        prev_ = curr_item->next_;
                    }
                    else
                    {
                        previous_item = prev_;
                        while (previous_item->next_ != curr_item)
                        {
                            previous_item = previous_item->next_;
                        }
                        previous_item->next_ = curr_item->next_;
                    }
                }

                curr_item->next_ = start_;
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

        for (PoolType* iter = prev_; iter; iter = iter->next_)
        {
            if (iter == to_find)
            {
                iter->PoolDeallocate();

                if (last)
                {
                    last->next_ = iter->next_;
                }
                else
                {
                    prev_ = iter->next_;
                }
                iter->next_ = start_;
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
        for (PoolType* iter = this->prev_; iter; iter = iter->next_)
        {
            if (iter == to_find)
            {
                if (last)
                {
                    last->next_ = iter->next_;
                }
                else
                {
                    this->prev_ = iter->next_;
                }
                iter->next_ = nullptr;
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
