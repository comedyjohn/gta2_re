# Phase 5: Ownership and memory

**Goal:** every object has one clear owner, lifetimes are expressed in types, and a reference to an
object that has been freed and reused is detected instead of silently read. That last point is the
root cause of a whole class of bugs in the original: a ped's pointer to "its" car still pointing at a
pool slot that now holds a different car.

**Depends on:** phase 3. **Blocks:** phase 6.

**Exit criteria**

- [ ] One pool template (`ObjectPool`) for all 24 pools; `Pool.hpp`'s `PoolBasic`/`Pool` gone.
- [ ] References between pooled objects are `Handle<T>`; no raw pointer member points into a pool
      (except short-lived locals and the pool's own internals).
- [ ] `PTR32` and `platform/ptr32.cpp` gone; the script references objects by handle.
- [ ] 0 findings for `cppcoreguidelines-owning-memory`, `-no-malloc`, `-special-member-functions`,
      `bugprone-undefined-memory-manipulation`, `cppcoreguidelines-pro-type-member-init`.
- [ ] No `new`/`delete`/`malloc`/`free` outside `core/` allocators; no `memcpy`/`memset` on non-trivial
      types (218 calls today, to be reviewed one by one).
- [ ] The corpus runs clean under ASan+UBSan; every commit hash-neutral except `Fix:` commits.

## 5.1 The pools today

Two templates in `Pool.hpp`:

- `PoolBasic<T, N>`: an array of `N` objects and an intrusive free list through `T::mpNext`. `Allocate()`
  pops the head and calls `T::PoolAllocate()`; `DeAllocate()` calls `T::PoolDeallocate()` and pushes.
- `Pool<T, N>`: the same plus a used list (also through `mpNext`), so it can be iterated.
  `Allocate()` pushes onto the front of the used list (newest first); `AllocateAppend()` walks to the
  end and appends (oldest first). `GetFirstUsed_4B8FD0()` starts an iteration.

Instantiations: `Pool<Ped, 200>`, `Pool<Car_BC, 306>`, `Pool<Object_2C, 3825>`, `Pool<Light_28, 3000>`,
`Pool<Particle_4C, 500>`, `Pool<Weapon_30, 255>`, `Pool<TileAnim_18, 50>`, `Pool<miss2_0x11C, 8>`,
`Pool<ExplodingScore_50, 3>`, and `PoolBasic` of `Sprite` (5,031), `SpriteShape_4C` (5,031),
`GridCell_C` (6,000), `GridSpriteLink_8` (4,096), `Object_8` (1,101), `Char_B4` (400), `Object_3C`
(385), `CarPhysics_B0` (306), `CarAI_78` (306), `AttachedSprite_18` (300), `Char_8` (100), `Taxi_8` (100),
`DoorData_10` (44), `Trailer` (10), `ScriptStackFrame_C`.

Behaviour that must survive the refactor exactly:

1. **Iteration order.** Services iterate the used list, so the update order of peds, cars and objects
   is newest-first (or oldest-first for `AllocateAppend`). Changing it changes the game.
2. **Which slot gets reused.** The free list is LIFO: the most recently freed slot is allocated next.
   Some code may depend on that (an index or address compared later).
3. **What a reused object contains.** `PoolAllocate()` resets only some fields; the rest keep the
   previous occupant's values, and the game may read them.
4. **Exhaustion.** `Allocate()` on an empty free list dereferences `nullptr`. Callers that care check
   `has_pStart_48A8F0()` first; others crash, as the original would.

## 5.2 `ObjectPool<T, N>`

```cpp
template <class T, std::size_t N>
class ObjectPool
{
  public:
    [[nodiscard]] Handle<T> Allocate();        // LIFO slot reuse, pushed to the front of the used list
    [[nodiscard]] Handle<T> AllocateAppend();  // the same, appended to the end of the used list
    void Free(Handle<T> h);
    [[nodiscard]] T* Get(Handle<T> h);         // nullptr if h is stale or null
    [[nodiscard]] bool Full() const;
    UsedRange<T> Used();                       // iterates in today's used-list order
    std::size_t IndexOf(const T& obj) const;   // for the state hash and debugging
  private:
    std::array<T, N> slots_;                   // objects live for the pool's lifetime (5.4)
    std::array<u16, N> generation_{};
    std::array<u16, N> next_;                  // free list and used list, by index
    ...
};

template <class T>
struct Handle { u16 index = kNull; u16 generation = 0; };  // 4 bytes, fits a PTR32 slot
```

- [ ] Write it in `core/object_pool.hpp` with unit tests that pin the four behaviours of 5.1 (order,
      reuse, contents, exhaustion), written against the old `Pool` first so both pass the same tests.
- [ ] Switch the pools over one by one (one commit each), keeping `T*`-based call sites working through
      a temporary `ObjectPool::GetPointer()` bridge. Replays after each.
- [ ] `Allocate()` on a full pool calls `FatalError` with the pool name instead of crashing (`Fix:`
      only if a replay reaches it, which would mean the original crashed there).

## 5.3 References become handles

About 160 data members hold raw pointers to pooled objects (36 `Ped*`, 37 `Car_BC*`, 51 `Object_2C*`,
22 `Sprite*`, 6 `Light_28*`, 4 `Weapon_30*`, and a few more), plus globals and the script's `PTR32`
slots. Each becomes a `Handle<T>`.

- [ ] Order: the ones with the most pointer-chasing first, because they are where stale references
      bite: ped ↔ car (driver, passengers, the car a ped is entering), ped → target ped, police crew →
      target, ped group members, player → ped, then objects, sprites, lights, weapons.
- [ ] At each use: `if (Car* car = cars.Get(ped.car_)) { ... }`. Where the original read a stale
      pointer, `Get()` now returns `nullptr`: that is a behaviour difference, and the replays show
      whether it ever happens. If it does, decide per case (keep the stale read explicitly with
      `GetEvenIfStale()` and a comment, or `Fix:`), and log it in `port/refactor/stale-references.md`.
- [ ] The sprite's owner union (phase 3.5) holds a handle plus the owner kind.
- [ ] `PTR32`: the script's object slots store `Handle` (4 bytes, the slot size). `platform/ptr32.cpp`'s
      handle table and the 32/64-bit `#if` go away, and the script data block no longer differs between
      32- and 64-bit builds.

## 5.4 Construction, reset, destruction

Today pooled objects are constructed once with the pool and then recycled through `PoolAllocate()` and
`PoolDeallocate()`. Keep that model (it is also the fastest), and make it explicit:

- [ ] Rename the hooks to `OnAllocate()`/`OnFree()` and document, per type, which fields they reset and
      which carry over (task 5.1, point 3). Fields that carry over and are read before being written are
      listed: each is either reset (a `Fix:` if the replays change) or documented as intended.
- [ ] Objects outside pools (the managers `Game_0x40` creates, about 50 of them): `std::unique_ptr` owned
      by their creator now, by `World` in phase 6. `GTA2_DELETE_AND_NULL` goes.
- [ ] Rule of zero for everything else; `= delete` copy and move on types that must not be copied
      (managers, pools, anything with a handle to itself).

## 5.5 Raw memory

- [ ] `new`/`delete` (179/73): mostly the managers in `Game_0x40::Game_0x40` and the network code;
      `std::make_unique` or members by value.
- [ ] `malloc`/`free` family (50): the style's and map's buffers (`gtx_0x106C`, `map_0x370`), the light
      grid, `Aligned_malloc_4FE510`: `std::vector` or `std::unique_ptr<T[]>`; alignment through
      `alignas` types.
- [ ] `memcpy`/`memset`/`memmove` (218), by kind:

| Kind | Becomes |
|---|---|
| Clearing a trivially copyable struct or array | `= {}` or `std::ranges::fill` |
| Copying a trivially copyable struct | assignment |
| Copying a non-trivial object (`PedManager::ClonePed_470F90` copies a whole `Ped`) | a `Clone()` that copies the fields the original copied (including the pool links it then repairs), written out |
| Copying bytes between buffers (files, network, script data) | `std::ranges::copy` on spans, or `BinaryReader`/`Writer` |

## 5.6 Lifetime checks

- [ ] Debug builds poison freed pool slots (fill with `0xDD` and bump the generation), so a raw pointer
      that survived the conversion shows up under the replays.
- [ ] ASan pool annotations (`ASAN_POISON_MEMORY_REGION` on free slots) in the ASan preset, so ASan
      reports use of a freed pool object like a heap use-after-free.

## Risks

| Risk | Mitigation |
|---|---|
| Iteration or reuse order changes | `ObjectPool` keeps both; tests pin them; replays |
| A stale reference the game relied on now reads `nullptr` | Replays find it; `GetEvenIfStale()` keeps it explicitly; documented |
| Handle checks cost time in hot loops | Measure with the phase 7 benchmark; `Get()` is an index and a compare |
| The script's slot layout changes | `Handle` is 4 bytes like `PTR32`; the script reader (phase 3.7) owns the layout |
