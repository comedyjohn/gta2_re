// Pool and PoolBasic, the fixed-size object pools every game object lives in. Characterisation:
// the order of the lists is behaviour (objects are updated in used-list order).
#include <doctest.h>

#include "Pool.hpp"

namespace
{
struct Item
{
    Item* mpNext;
    int allocations = 0;
    int deallocations = 0;
    bool remove = false;

    void PoolAllocate()
    {
        allocations++;
    }
    void PoolDeallocate()
    {
        deallocations++;
    }
    bool PoolUpdate()
    {
        return remove;
    }
};
} // namespace

namespace
{
TEST_CASE("PoolBasic hands out items in array order and reuses the last freed first")
{
    PoolBasic<Item, 4> pool;
    Item* a = pool.Allocate();
    Item* b = pool.Allocate();
    CHECK(a == &pool.pool_[0]);
    CHECK(b == &pool.pool_[1]);
    CHECK(a->allocations == 1);
    pool.DeAllocate(a);
    CHECK(a->deallocations == 1);
    CHECK(pool.Allocate() == a);
    pool.Allocate();
    pool.Allocate();
    CHECK(pool.head_ == nullptr); // full
}

TEST_CASE("Pool keeps a used list, newest first")
{
    Pool<Item, 4> pool;
    Item* a = pool.Allocate();
    Item* b = pool.Allocate();
    Item* c = pool.Allocate();
    CHECK(pool.GetFirstUsed() == c);
    CHECK(c->mpNext == b);
    CHECK(b->mpNext == a);
    CHECK(a->mpNext == nullptr);
}

TEST_CASE("Pool::AllocateAppend adds at the end of the used list")
{
    Pool<Item, 4> pool;
    Item* a = pool.Allocate();
    Item* b = pool.AllocateAppend();
    CHECK(pool.GetFirstUsed() == a);
    CHECK(a->mpNext == b);
    CHECK(b->mpNext == nullptr);
}

TEST_CASE("Pool::UpdatePool removes the items whose update says so, keeping the order")
{
    Pool<Item, 4> pool;
    Item* a = pool.Allocate();
    Item* b = pool.Allocate();
    Item* c = pool.Allocate();
    b->remove = true;
    pool.UpdatePool();
    CHECK(pool.field_X_count == 3); // counted before removal
    CHECK(pool.GetFirstUsed() == c);
    CHECK(c->mpNext == a);
    CHECK(b->deallocations == 1);
    CHECK(pool.Allocate() == b); // back on the free list
}
} // namespace
