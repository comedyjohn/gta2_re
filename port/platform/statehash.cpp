// The state hash (statehash.h). Rules that keep it the same between runs and between builds:
// - values, never addresses: a pointer into a pool is hashed as the pool and the index in it;
// - field by field, never whole structures (padding, and layouts differ between 32 and 64 bits);
// - pools in array order, plus the order of their used lists, which is the update order;
// - only fields the game sets: some are never written (PolicePursuitTarget_7C::field_40 and
//   field_54), and would hash whatever the heap held (AddressSanitizer's fill pattern, say).
// FNV-1a 64 over little-endian bytes. The same walk can write every value out as text instead.
#include "statehash.h"

#include "CarAI_78.hpp"
#include "CarPhysics_B0.hpp"
#include "Car_BC.hpp"
#include "Char_Pool.hpp"
#include "Cranes.hpp"
#include "Crushers.hpp"
#include "Door_4D4.hpp"
#include "EmergencyCrewPool_1E0.hpp"
#include "Explosion_30.hpp"
#include "Game_0x40.hpp"
#include "Generators.hpp"
#include "Hud.hpp"
#include "Light_1D4CC.hpp"
#include "Object_2C_Pool.hpp"
#include "Object_3C_Pool.hpp"
#include "Object_5C.hpp"
#include "Object_8_Pool.hpp"
#include "Particle_8.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "Police_38.hpp"
#include "Police_7B8.hpp"
#include "ScriptManager_C1EA8.hpp"
#include "Weapon_8.hpp"
#include "char.hpp"
#include "miss2_0x11C.hpp"
#include "rng.hpp"
#include "sprite.hpp"

#include <stdio.h>
#include <string.h>

#include <vector>

namespace
{
class Hasher
{
  public:
    explicit Hasher(std::string* dump, const char* component) : mDump(dump), mComponent(component)
    {
    }

    void Add(const char* name, int64_t v)
    {
        Bytes(&v, sizeof(v));
        if (mDump)
        {
            char line[256];
            snprintf(line, sizeof(line), "%s%s%s %lld\n", mComponent, mPrefix.c_str(), name, static_cast<long long>(v));
            *mDump += line;
        }
    }

    // A block hashed as it is (its bytes are data, not pointers or padding)
    void Block(const char* name, const void* p, size_t len)
    {
        const uint64_t before = mHash;
        mHash = 1469598103934665603ULL;
        Bytes(p, len);
        const uint64_t block = mHash;
        mHash = before;
        Add(name, static_cast<int64_t>(block));
    }

    // Values that follow belong to item `idx` of `group` (in the text dump)
    void Item(const char* group, int idx)
    {
        char prefix[64];
        snprintf(prefix, sizeof(prefix), ".%s[%d].", group, idx);
        mPrefix = prefix;
    }

    void EndItem()
    {
        mPrefix = ".";
    }

    uint64_t Value() const
    {
        return mHash;
    }

  private:
    void Bytes(const void* p, size_t len)
    {
        const unsigned char* b = static_cast<const unsigned char*>(p);
        for (size_t i = 0; i < len; i++)
        {
            mHash ^= b[i];
            mHash *= 1099511628211ULL;
        }
    }

    uint64_t mHash = 1469598103934665603ULL;
    std::string* mDump;
    const char* mComponent;
    std::string mPrefix = ".";
};

// Turns a pointer into "which pool, which item": (pool << 16) | index; null is -1, a pointer into
// none of the pools -2
class PoolMap
{
  public:
    template<typename T, size_t N>
    void Add(T (&items)[N], int id)
    {
        mRanges.push_back({reinterpret_cast<const char*>(&items[0]), reinterpret_cast<const char*>(&items[0] + N), sizeof(T), id});
    }

    int64_t Of(const void* p) const
    {
        if (!p)
        {
            return -1;
        }
        const char* c = static_cast<const char*>(p);
        for (const Range& r : mRanges)
        {
            if (c >= r.begin && c < r.end)
            {
                return (static_cast<int64_t>(r.id) << 16) | ((c - r.begin) / r.stride);
            }
        }
        return -2;
    }

  private:
    struct Range
    {
        const char* begin;
        const char* end;
        size_t stride;
        int id;
    };
    std::vector<Range> mRanges;
};

enum PoolId
{
    kSprites = 1,
    kPeds,
    kChars,
    kCars,
    kCarPhysics,
    kCarAI,
    kTrailers,
    kObjects2C,
    kObjects3C,
    kObjects8,
    kThreads,
    kWeapons,
    kLights,
    kDoors,
    kCranes,
    kCrushers,
    kGenerators,
    kArrows,
    kAttachedSprites,
    kParticles,
    kExplosions40,
    kExplosions20,
};

PoolMap MakePoolMap()
{
    PoolMap m;
    m.Add(gSpritePool_703818->field_0_pool.pool_, kSprites);
    m.Add(gPedPool_6787B8->field_0_pool.pool_, kPeds);
    m.Add(gChar_B4_Pool_6FDB44->field_0_pool.pool_, kChars);
    m.Add(gCar_BC_Pool_67792C->field_0_pool.pool_, kCars);
    m.Add(gCarPhysicsPool_6FE3E0->pool_, kCarPhysics);
    m.Add(gCarAI_78_Pool_677CF8->field_0_pool.pool_, kCarAI);
    m.Add(gTrailerPool_66AC80->field_0_pool.pool_, kTrailers);
    m.Add(gObject_2C_Pool_6F8F80->pool_, kObjects2C);
    m.Add(gObject_3C_Pool_6F8F7C->pool_, kObjects3C);
    m.Add(gObject_8_Pool_6F8F78->pool_, kObjects8);
    m.Add(miss2_0x11C_Pool_6F8064->field_0_pool.pool_, kThreads);
    m.Add(gWeapon_30_Pool_707014->field_0_pool.pool_, kWeapons);
    m.Add(gLight_1D4CC_6F5520->field_0_pool.pool_, kLights);
    m.Add(gDoor_4D4_67BD2C->field_0_doors, kDoors);
    m.Add(gCranePool_D9C_679FD4->field_0_cranes, kCranes);
    // field_0_crushers[3] is private, and the pool's first member
    m.Add(*reinterpret_cast<Crusher_30(*)[3]>(gCrusherPool_94_67A830), kCrushers);
    m.Add(gGeneratorPool_14AC_67E5D0->field_0_pool, kGenerators);
    m.Add(gHud_2B00_706620->field_1F18_arrows.field_0_array, kArrows);
    m.Add(gAttachedSpritePool_703B80->field_0_pool.pool_, kAttachedSprites);
    m.Add(gParticle_4C_Pool_6FD5E4->pool_, kParticles);
    m.Add(gExplosionPool_7A8_6FD5F0->field_0_explosions, kExplosions40);
    m.Add(gExplosionPool_3D4_6FD5EC->field_0_explosions, kExplosions20);
    return m;
}

// Which items of a pool are in use: everything not on its free list
template<typename T, s32 N>
std::vector<bool> UsedItems(PoolBasic<T, N>& pool)
{
    std::vector<bool> used(N, true);
    for (T* p = pool.head_; p; p = p->mpNext)
    {
        used[p - pool.pool_] = false;
    }
    return used;
}

template<typename T, s32 N>
std::vector<bool> UsedItems(Pool<T, N>& pool)
{
    std::vector<bool> used(N, true);
    for (T* p = pool.start_; p; p = p->mpNext)
    {
        used[p - pool.pool_] = false;
    }
    return used;
}

// The used list of a Pool, in order (the order the game updates its items in)
template<typename T, s32 N>
void AddUsedOrder(Hasher& h, const PoolMap& pools, Pool<T, N>& pool)
{
    int count = 0;
    for (T* p = pool.prev_; p && count <= N; p = p->mpNext, count++)
    {
        h.Add("used_list", pools.Of(p));
    }
    h.Add("used_count", count);
}

template<typename T, s32 N>
int CountUsed(PoolBasic<T, N>& pool)
{
    int count = 0;
    for (bool used : UsedItems(pool))
    {
        count += used;
    }
    return count;
}

void AddPoint(Hasher& h, const char* name, const Fix16Point& p)
{
    char n[64];
    snprintf(n, sizeof(n), "%s.x", name);
    h.Add(n, p.x.mValue);
    snprintf(n, sizeof(n), "%s.y", name);
    h.Add(n, p.y.mValue);
}

uint64_t HashRng(std::string* dump)
{
    Hasher h(dump, "rng");
    h.Add("count", gpRng_67AB34->rng_);
    h.Add("rnd", gpRng_67AB34->rnd_);
    return h.Value();
}

uint64_t HashSprites(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "sprites");
    PoolBasic<Sprite, 5031>& pool = gSpritePool_703818->field_0_pool;
    const std::vector<bool> used = UsedItems(pool);
    for (int i = 0; i < 5031; i++)
    {
        if (!used[i])
        {
            continue;
        }
        const Sprite& s = pool.pool_[i];
        h.Item("sprite", i);
        h.Add("index", i);
        h.Add("x", s.field_14_xy.x.mValue);
        h.Add("y", s.field_14_xy.y.mValue);
        h.Add("z", s.field_1C_zpos.mValue);
        h.Add("rotation", s.field_0_rotation.rValue);
        h.Add("type", s.field_30_sprite_type_enum);
        h.Add("instance_id", s.field_20_instance_id);
        h.Add("frame", s.field_22_type_sprite_idx);
        h.Add("owner", pools.Of(s.field_8_pSprite));
    }
    h.EndItem();
    return h.Value();
}

uint64_t HashPeds(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "peds");
    Pool<Ped, 200>& pool = gPedPool_6787B8->field_0_pool;
    AddUsedOrder(h, pools, pool);
    const std::vector<bool> used = UsedItems(pool);
    for (int i = 0; i < 200; i++)
    {
        if (!used[i])
        {
            continue;
        }
        const Ped& p = pool.pool_[i];
        h.Item("ped", i);
        h.Add("index", i);
        h.Add("id", p.field_200_id);
        h.Add("health", p.field_216_health);
        h.Add("objective", p.field_258_objective);
        h.Add("internal_objective", p.field_25C_internal_objective);
        h.Add("objective_status", p.field_225_objective_status);
        h.Add("state_1", p.field_278_ped_state_1);
        h.Add("state_2", p.field_27C_ped_state_2);
        h.Add("occupation", p.field_240_occupation);
        h.Add("wanted_points", p.field_20A_wanted_points);
        h.Add("threat_search", p.field_288_threat_search);
        h.Add("threat_reaction", p.field_28C_threat_reaction);
        h.Add("car", pools.Of(p.field_16C_car));
        h.Add("char", pools.Of(p.field_168_game_object));
        h.Add("target_ped", pools.Of(p.field_148_objective_target_ped));
        h.Add("target_car", pools.Of(p.field_150_target_objective_car));
        h.Add("weapon", pools.Of(p.field_170_selected_weapon));
    }

    PoolBasic<Char_B4, 400>& chars = gChar_B4_Pool_6FDB44->field_0_pool;
    const std::vector<bool> usedChars = UsedItems(chars);
    for (int i = 0; i < 400; i++)
    {
        if (!usedChars[i])
        {
            continue;
        }
        const Char_B4& c = chars.pool_[i];
        h.Item("char", i);
        h.Add("index", i);
        h.Add("ped_state_1", c.field_8_ped_state_1);
        h.Add("ped_state_2", c.field_C_ped_state_2);
        h.Add("char_state", c.field_10_char_state);
        h.Add("velocity", c.field_38_velocity.mValue);
        h.Add("rotation", c.field_40_rotation.rValue);
        h.Add("animation_state", c.field_6C_animation_state);
        h.Add("animation_frame", c.field_68_animation_frame);
        h.Add("sprite", pools.Of(c.field_80_sprite_ptr));
    }
    h.EndItem();
    return h.Value();
}

uint64_t HashCars(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "cars");
    Pool<Car_BC, 306>& pool = gCar_BC_Pool_67792C->field_0_pool;
    AddUsedOrder(h, pools, pool);
    const std::vector<bool> used = UsedItems(pool);
    for (int i = 0; i < 306; i++)
    {
        if (!used[i])
        {
            continue;
        }
        const Car_BC& c = pool.pool_[i];
        h.Item("car", i);
        h.Add("index", i);
        h.Add("id", c.field_6C_car_id);
        h.Add("model", c.field_84_car_info_idx);
        h.Add("damage", c.field_74_damage);
        h.Add("flags", c.field_78_flags);
        h.Add("kind", c.field_A0_car_kind);
        h.Add("despawn_status", c.field_88_despawn_status);
        h.Add("engine_status", c.field_9C_engine_status);
        h.Add("driver", pools.Of(c.field_54_driver));
        h.Add("sprite", pools.Of(c.field_50_car_sprite));
        h.Add("physics", pools.Of(c.field_58_physics));
        h.Add("ai", pools.Of(c.field_5C_AI));
        if (const CarPhysics_B0* ph = c.field_58_physics)
        {
            AddPoint(h, "velocity", ph->field_40_linvel_1);
            h.Add("angular_velocity", ph->field_74_ang_vel_rad.mValue);
            h.Add("z", ph->field_68_z_pos.mValue);
            h.Add("z_velocity", ph->field_70_z_vel.mValue);
            h.Add("theta", ph->field_58_theta.rValue);
            h.Add("physics_state", ph->field_8C_state);
            h.Add("gas", ph->field_60_gas_pedal.mValue);
            h.Add("brake", ph->field_64_brake_pressure.mValue);
        }
    }
    h.EndItem();
    // How many cars of each kind exist: decides which kind the next spawn is (CanAllocateCarOfKind)
    const CarManager_6C& m = *gCarManager_6C_677930;
    h.Add("count_recycled", m.field_28_recycled_cars);
    h.Add("count_paramedic", m.field_2C_paramedic_cars);
    h.Add("count_firefighter", m.field_30_firefighter_cars);
    h.Add("count_unit", m.field_34_unit_cars);
    h.Add("count_roadblock", m.field_38_roadblock_cars);
    h.Add("count_mission", m.field_3C_mission_cars);
    h.Add("count_proto_recycled", m.field_40_proto_recycled_cars);
    h.Add("count_parked", m.field_44_parked_cars);
    h.Add("count_kind10", m.field_48_kind10_cars);
    return h.Value();
}

uint64_t HashObjects(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "objects");
    Pool<Object_2C, 3825>& pool = *gObject_2C_Pool_6F8F80;
    AddUsedOrder(h, pools, pool);
    const std::vector<bool> used = UsedItems(pool);
    for (int i = 0; i < 3825; i++)
    {
        if (!used[i])
        {
            continue;
        }
        const Object_2C& o = pool.pool_[i];
        h.Item("object", i);
        h.Add("index", i);
        h.Add("id", o.field_14_id);
        h.Add("model", o.field_18_model);
        h.Add("sprite", pools.Of(o.field_4));
        h.Add("list_state", o.field_20_pool_list_state);
        h.Add("removal_state", o.field_25_removal_state);
        h.Add("touched", o.field_24_touched_result);
    }
    h.EndItem();
    h.Add("objects_3c", CountUsed(*gObject_3C_Pool_6F8F7C));
    h.Add("objects_8", CountUsed(*gObject_8_Pool_6F8F78));
    int particles = 0;
    for (bool u : UsedItems(*gParticle_4C_Pool_6FD5E4))
    {
        particles += u;
    }
    h.Add("particles", particles);
    for (int i = 0; i < 40; i++)
    {
        h.Add("explosion_used", gExplosionPool_7A8_6FD5F0->field_780_bUsed[i]);
    }
    for (int i = 0; i < 20; i++)
    {
        h.Add("explosion_used", gExplosionPool_3D4_6FD5EC->field_3C0_bUsed[i]);
    }
    return h.Value();
}

uint64_t HashPlayers(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "players");
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        Player* p = gGame_0x40_67E008->field_4_players[i];
        h.Item("player", i);
        h.Add("present", p != nullptr);
        if (!p)
        {
            continue;
        }
        h.Add("ped", pools.Of(p->field_2C4_player_ped));
        h.Add("inputs", p->field_4_inputs);
        h.Add("money", p->field_2D4_scores.field_0_money.field_0_value);
        h.Add("multiplier", p->field_6BC_multipliers.field_0_value);
        h.Add("lives", p->field_684_lives.field_0_value);
        h.Add("wasted_or_busted", p->field_28_wasted_or_busted);
        h.Add("death_countdown", p->field_2C_death_countdown);
        h.Add("weapon", p->field_788_curr_weapon_idx);
        for (int t = 0; t < 17; t++)
        {
            h.Add("power_up_timer", p->field_6F4_power_up_timers[t]);
        }
    }
    h.EndItem();
    h.Add("game_state", gGame_0x40_67E008->field_0_game_state);
    return h.Value();
}

uint64_t HashScript(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "script");
    Pool<miss2_0x11C, 8>& threads = miss2_0x11C_Pool_6F8064->field_0_pool;
    AddUsedOrder(h, pools, threads);
    const std::vector<bool> used = UsedItems(threads);
    for (int i = 0; i < 8; i++)
    {
        if (!used[i])
        {
            continue;
        }
        const miss2_0x11C& t = threads.pool_[i];
        h.Item("thread", i);
        h.Add("index", i);
        h.Add("next_cmd", t.field_4_next_cmd);
        h.Add("cond_result", t.field_8_cond_result);
        h.Add("cmd_initialised", t.field_C_cmd_initialised);
        h.Add("phone_timer", t.field_E_phone_timer);
        h.Add("stopped", t.field_10_stopped);
        h.Add("exec_flag", t.field_12_exec_flag);
        h.Add("thread_id", t.field_11A_thread_id);
    }
    h.EndItem();

    // The script's data: its variables and the objects its commands created live in it. The
    // pointers among them become pool indices, everything else is hashed as it is.
    ScriptManager_C1EA8* sm = gScriptManager_6F8060;
    h.Block("base_pointers", sm->field_46C_base_pointers, sizeof(sm->field_46C_base_pointers));
    static std::vector<unsigned char> data;
    data.assign(sm->field_334C_script_data, sm->field_334C_script_data + sizeof(sm->field_334C_script_data));
    struct Ctx
    {
        unsigned char* data;
        const PoolMap* pools;
    } ctx {data.data(), &pools};
    Ptr32_SlotsIn(
        sm->field_334C_script_data,
        sizeof(sm->field_334C_script_data),
        [](uint32_t offset, void* c)
        {
            Ctx* ctx = static_cast<Ctx*>(c);
            u32 handle;
            memcpy(&handle, ctx->data + offset, sizeof(handle));
            const u32 idx = static_cast<u32>(ctx->pools->Of(Ptr32_FromHandle(handle)));
            memcpy(ctx->data + offset, &idx, sizeof(idx));
        },
        &ctx);
    h.Block("data", data.data(), data.size());
    if (dump)
    {
        // Per variable in the dump: each command's first 4 data bytes (counters and created objects)
        for (int i = 0; i < 6000; i++)
        {
            const u16 offset = sm->field_46C_base_pointers[i];
            if (offset && offset + 12U <= data.size())
            {
                s32 value;
                memcpy(&value, &data[offset + 8], sizeof(value));
                h.Item("cmd", i);
                h.Add("value", value);
            }
        }
        h.EndItem();
    }
    return h.Value();
}

uint64_t HashPolice(std::string* dump, const PoolMap& pools)
{
    Hasher h(dump, "police");
    Police_7B8* p = gPolice_7B8_6FEE40;
    h.Add("present", p != nullptr);
    if (!p)
    {
        return h.Value();
    }
    h.Add("max_wanted_level", p->field_654_max_wanted_level);
    h.Add("police_cars", p->field_658_police_car_count);
    h.Add("highest_crew_type", p->field_65C_highest_crew_type_in_pursuit);
    h.Add("max_wanted_stars", p->field_660_max_wanted_stars);
    for (int i = 0; i < MAX_POLICE_CREWS; i++)
    {
        const PoliceCrew_38& c = p->field_4_cop_crew[i];
        h.Item("crew", i);
        h.Add("used", c.field_1C_used);
        if (!c.field_1C_used)
        {
            continue;
        }
        h.Add("id", c.field_0_id);
        h.Add("target_x", c.field_2_targ_x);
        h.Add("target_y", c.field_3_targ_y);
        h.Add("target_z", c.field_4_targ_z);
        h.Add("field_8", c.field_8.mValue);
        h.Add("field_C", c.field_C.mValue);
        h.Add("emergency_crew", c.field_10_subObj ? c.field_10_subObj - gEmergencyCrewPool_706280->field_0_slots : -1);
        h.Add("pursuit", c.field_14_pPursuitTarget ? c.field_14_pPursuitTarget - p->field_464_pursuit_targets : -1);
        h.Add("field_18", c.field_18);
        h.Add("kind", c.field_20_crew_kind);
        h.Add("state", c.field_24_state);
        h.Add("field_28", c.field_28);
        h.Add("counted", c.field_29_bCountedInPoliceCount);
        h.Add("field_2A", c.field_2A);
        h.Add("ped", pools.Of(reinterpret_cast<const void*>(c.field_30)));
        h.Add("dismissed", c.field_34_is_dismissed);
        h.Add("field_35", c.field_35);
    }
    for (int i = 0; i < MAX_PURSUIT_TARGETS; i++)
    {
        const PolicePursuitTarget_7C& t = p->field_464_pursuit_targets[i];
        h.Item("pursuit", i);
        h.Add("criminal", pools.Of(t.field_0_criminal_ped));
        h.Add("wanted_level", t.field_4_wanted_level);
        h.Add("state", t.field_8_state);
        h.Add("timer", t.field_C_timer);
        h.Add("field_E", t.field_E);
        h.Add("x", t.field_10_x.mValue);
        h.Add("y", t.field_14_y.mValue);
        h.Add("z", t.field_18_z.mValue);
        h.Add("gave_up", t.field_1C_crew_gave_up);
        for (PoliceCrew_38* crew : t.field_20_crews)
        {
            h.Add("crew", crew ? crew - p->field_4_cop_crew : -1);
        }
        h.Add("police_crews", t.field_70_num_police_crews);
        h.Add("target_police_crews", t.field_71_target_police_crews);
        h.Add("swat_crews", t.field_72_num_swat_crews);
        h.Add("fbi_crews", t.field_73_num_fbi_crews);
        h.Add("army_crews", t.field_74_num_army_crews);
        h.Add("crews", t.field_75_num_crews);
        h.Add("actively_chased", t.field_78_is_actively_chased);
        h.Add("wanted_timer", t.field_7A_wanted_timer);
    }
    h.EndItem();
    return h.Value();
}

StateHashes Compute(std::string* dump)
{
    const PoolMap pools = MakePoolMap();
    StateHashes s;
    s.rng = HashRng(dump);
    s.sprites = HashSprites(dump, pools);
    s.peds = HashPeds(dump, pools);
    s.cars = HashCars(dump, pools);
    s.objects = HashObjects(dump, pools);
    s.players = HashPlayers(dump, pools);
    s.script = HashScript(dump, pools);
    s.police = HashPolice(dump, pools);

    Hasher total(nullptr, "total");
    for (uint64_t c : {s.rng, s.sprites, s.peds, s.cars, s.objects, s.players, s.script, s.police})
    {
        total.Add("", static_cast<int64_t>(c));
    }
    s.total = total.Value();
    return s;
}
} // namespace

StateHashes port_state_hash()
{
    return Compute(nullptr);
}

std::string port_state_dump()
{
    std::string dump;
    Compute(&dump);
    return dump;
}
