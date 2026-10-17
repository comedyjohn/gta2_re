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
        PoolType* pIter = &pool_[0];
        for (s32 i = 0; i < PoolSize - 1; i++)
        {
            pIter->mpNext = pIter + 1;
            pIter++;
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
        PoolType* pNew = head_;
        head_ = pNew->mpNext;
        pNew->PoolAllocate();
        return pNew;
    }

    void DeAllocate(PoolType* item)
    {
        item->PoolDeallocate();
        item->mpNext = head_;
        head_ = item;
    }

    PoolType* UnlinkAndReturnNext(PoolType* old_item)
    {
        PoolType* pNewItem = old_item->mpNext;
        old_item->mpNext = this->head_;
        this->head_ = old_item;
        return pNewItem;
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
        PoolType* pIter = pool_;
        for (s32 i = 0; i < PoolSize - 1; i++)
        {
            pIter->mpNext = pIter + 1;
            pIter++;
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

        PoolType* pPreviousItem = nullptr;
        PoolType* pCurrItem = prev_;

        while (pCurrItem)
        {
            ++field_x_count_;

            PoolType* pNext = pCurrItem->mpNext;

            if (pCurrItem->PoolUpdate())
            {
                pCurrItem->PoolDeallocate();

                if (pPreviousItem && pPreviousItem->mpNext != pCurrItem)
                {
                    pPreviousItem = nullptr;
                }

                if (pPreviousItem)
                {
                    pPreviousItem->mpNext = pCurrItem->mpNext;
                }
                else
                {
                    if (prev_ == pCurrItem)
                    {
                        prev_ = pCurrItem->mpNext;
                    }
                    else
                    {
                        pPreviousItem = prev_;
                        while (pPreviousItem->mpNext != pCurrItem)
                        {
                            pPreviousItem = pPreviousItem->mpNext;
                        }
                        pPreviousItem->mpNext = pCurrItem->mpNext;
                    }
                }

                pCurrItem->mpNext = start_;
                start_ = pCurrItem;
            }
            else
            {
                pPreviousItem = pCurrItem;
            }

            pCurrItem = pNext;
        }
    }

    void UpdatePoolNoDeallocate()
    {
        field_x_count_ = 0;

        PoolType* pPreviousItem = nullptr;
        PoolType* pCurrItem = prev_;

        while (pCurrItem)
        {
            ++field_x_count_;

            PoolType* pNext = pCurrItem->mpNext;

            if (pCurrItem->PoolUpdate())
            {
                if (pPreviousItem && pPreviousItem->mpNext != pCurrItem)
                {
                    pPreviousItem = nullptr;
                }

                if (pPreviousItem)
                {
                    pPreviousItem->mpNext = pCurrItem->mpNext;
                }
                else
                {
                    if (prev_ == pCurrItem)
                    {
                        prev_ = pCurrItem->mpNext;
                    }
                    else
                    {
                        pPreviousItem = prev_;
                        while (pPreviousItem->mpNext != pCurrItem)
                        {
                            pPreviousItem = pPreviousItem->mpNext;
                        }
                        pPreviousItem->mpNext = pCurrItem->mpNext;
                    }
                }

                pCurrItem->mpNext = start_;
                start_ = pCurrItem;
            }
            else
            {
                pPreviousItem = pCurrItem;
            }

            pCurrItem = pNext;
        }
    }

    PoolType* FindAndDeAllocate(PoolType* to_find)
    {
        PoolType* pLast = nullptr;

        for (PoolType* pIter = prev_; pIter; pIter = pIter->mpNext)
        {
            if (pIter == to_find)
            {
                pIter->PoolDeallocate();

                if (pLast)
                {
                    pLast->mpNext = pIter->mpNext;
                }
                else
                {
                    prev_ = pIter->mpNext;
                }
                pIter->mpNext = start_;
                start_ = pIter;
                return to_find;
            }
            pLast = pIter;
        }
        return to_find;
    }

    void UnlinkFromActiveList(PoolType* to_find)
    {
        PoolType* pLast = nullptr;
        for (PoolType* pIter = this->prev_; pIter; pIter = pIter->mpNext)
        {
            if (pIter == to_find)
            {
                if (pLast)
                {
                    pLast->mpNext = pIter->mpNext;
                }
                else
                {
                    this->prev_ = pIter->mpNext;
                }
                pIter->mpNext = nullptr;
                return;
            }
            pLast = pIter;
        }
    }

    PoolType* start_;
    PoolType* prev_;
    PoolType pool_[PoolSize];

    // Unknown offsets due to sizeof(PoolType) varying per template
    s16 field_x_count_;
    s16 field_x_pad_;
};
