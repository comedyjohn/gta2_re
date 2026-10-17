#include "Weapon_8.hpp"
#include "Car_BC.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "error.hpp"

DEFINE_GLOBAL(Weapon_30_Pool*, gWeapon_30_Pool_707014, 0x707014);
DEFINE_GLOBAL(Weapon_8*, gWeapon_8_707018, 0x707018);
// Read only in Weapon_30.cpp, defined here: see the comment there.
DEFINE_GLOBAL_INIT(Ang16, word_706D5E, Ang16(48), 0x706D5E);

DEFINE_GLOBAL_ARRAY_INIT(u8,
                         max_ammo_capacity_5FF75C,
                         28,
                         0x5FF75C,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         1U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U,
                         99U);

u8 byte_5FF778[40] = {10U, 10U, 5U,  20U, 5U, 5U, 10U, 20U, 20U, 10U, 10U, 0U, 0U,  0U,  0U,  1U,  5U,   5U,  10U, 50U,
                      50U, 20U, 50U, 1U,  0U, 0U, 0U,  0U,  0U,  0U,  0U,  0U, 80U, 69U, 82U, 84U, 251U, 33U, 9U,  64U};

// 0x5e3c10
Weapon_30* Weapon_8::allocate_5E3C10(s32 weapon_kind, Ped* pPed, u8 ammo)
{
    Weapon_30* pNewWeap = gWeapon_30_Pool_707014->AllocateUnlinked_4CC9E0();
    pNewWeap->init_5DCD90();
    field_4_ref_count++;
    pNewWeap->set_idx_4CCA00(weapon_kind);
    pNewWeap->set_pPed_4CCA10(pPed);
    pNewWeap->add_ammo_5DCE20(ammo);
    return pNewWeap;
}

// 0x5e3cb0
void Weapon_8::deallocate_5E3CB0(Weapon_30* pWeapon)
{
    Weapon_30_Pool* pRoot = gWeapon_30_Pool_707014;
    pWeapon->PoolDeallocate();
    pWeapon->mpNext = pRoot->field_0_pool.start_;
    pRoot->field_0_pool.start_ = pWeapon;
    field_4_ref_count--;
}

// 0x5e3ce0
Weapon_30* Weapon_8::allocate_5E3CE0(s32 weapon_kind, Car_BC* pCar, u8 ammo)
{
    Weapon_30* pWeapon = gWeapon_30_Pool_707014->Allocate();
    pWeapon->init_5DCD90();
    pWeapon->set_idx_4CCA00(weapon_kind);
    pWeapon->set_car_4CCA20(pCar);
    pWeapon->add_ammo_5DCE20(ammo);
    return pWeapon;
}

// 0x5e3d20
Weapon_30* Weapon_8::find_5E3D20(Car_BC* pCar, s32 weapon_kind)
{
    Weapon_30* result = gWeapon_30_Pool_707014->get_next_4CC9B0();
    if (!result)
    {
        return nullptr;
    }

    // TODO: Can probably invert this
    while (result->field_14_car != pCar || result->field_1C_idx != weapon_kind)
    {
        result = result->mpNext;
        if (!result)
        {
            return nullptr;
        }
    }
    return result;
}

// 0x5e3d50
char_type Weapon_8::allocate_5E3D50(s32 weapon_kind, u8 ammo, Car_BC* pCar)
{
    char_type bAddedAmmo;

    Weapon_30* pWeapon = find_5E3D20(pCar, weapon_kind);
    if (pWeapon)
    {
        bAddedAmmo = pWeapon->add_ammo_capped_5DCE40(ammo);
    }
    else
    {
        pWeapon = allocate_5E3CE0(weapon_kind, pCar, ammo);
        bAddedAmmo = 1;
    }

    if (bAddedAmmo)
    {
        if (pCar->IsDrivenByPlayer_4118D0())
        {
            pCar->field_54_driver->field_15C_player->SetWeapon_564910(pWeapon);
            return bAddedAmmo;
        }
    }

    if (bAddedAmmo)
    {
        if (pCar->field_54_driver)
        {
            pCar->field_54_driver->field_178_car_weapon = gWeapon_8_707018->allocate_5E3C10(weapon_kind, pCar->field_54_driver, 99U);
            pCar->field_54_driver->field_178_car_weapon->field_14_car = pCar;
        }
    }

    return bAddedAmmo;
}

// 0x5e3df0
void Weapon_8::dealloc_car_weapon_5E3DF0(Car_BC* pCar)
{
    Weapon_30* pIter = gWeapon_30_Pool_707014->get_next_4CC9B0();
    while (pIter)
    {
        if (pIter->field_14_car == pCar)
        {
            Weapon_30* pOldIter = pIter;
            pIter = pIter->mpNext;
            gWeapon_30_Pool_707014->DeAllocate_4A4F20(pOldIter);
        }
        else
        {
            pIter = pIter->mpNext;
        }
    }
}

// 0x5e3e70
u8 Weapon_8::get_max_ammo_capacity_5E3E70(s32 weapon_kind)
{
    return max_ammo_capacity_5FF75C[weapon_kind];
}

// 0x5e3e80
u8 Weapon_8::get_defalt_ammo_5E3E80(s32 weapon_kind)
{
    return byte_5FF778[weapon_kind];
}

// 0x5e3e90
Weapon_8::Weapon_8()
{
    if (!gWeapon_30_Pool_707014)
    {
        gWeapon_30_Pool_707014 = new Weapon_30_Pool();
        if (!gWeapon_30_Pool_707014)
        {
            FatalError_4A38C0(Gta2Error::OutOfMemoryNewOperator, "weapon.cpp", 2428);
        }
    }
    field_4_ref_count = 0;
    field_0.ResetHead_4207E0();
}

// 0x5e3f60
Weapon_8::~Weapon_8()
{
    if (gWeapon_30_Pool_707014)
    {
        GTA2_DELETE_AND_NULL(gWeapon_30_Pool_707014);
    }
}

// 0x5E4090
Weapon_30_Pool::~Weapon_30_Pool() = default;
