#pragma once

#include "Particle_4C.hpp"
#include "core/ang16.hpp"
#include "core/fix16_point.hpp"
#include "core/function.hpp"
#include "core/pool.hpp"

class Fix16;
class Sprite;
class Sprite;
class Particle_4C;
class Object_2C;

class Particle_8
{
  public:
    Particle_4C* New_53E3C0(Fix16 speed_x,
                            Fix16 speed_y,
                            Fix16 field_10_value,
                            Fix16 additional_speed_x,
                            Fix16 additional_speed_y,
                            Fix16 field_1C_value);

    void ParticlesService_53E320();
    void EmitBloodBurst_53E450(Fix16 x, Fix16 y, Fix16 z, Ang16 ang);
    void SpawnBlood_53E880(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    void GunMuzzelFlash_53E970(Sprite* pSprite);
    void EmitWaterSplash_53F060(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, char_type bRandomRot);
    void SpawnParticleSprite_5405D0(Sprite* pSprite);
    void EmitElectricArcParticle(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 ang);
    void SpawnCigaretteSmokePuff_5406B0(Sprite* pSprite, char_type bUnknown);
    Particle_8();
    ~Particle_8();

    void EmitFireTruckSprayParticle_53FAE0(Sprite* pSprite);
    void EmitImpactParticles_53FE40(Fix16 x, Fix16 y, Fix16 z, Fix16Point dir);
    void EmitFlameStreamSegment_53F4C0(Sprite* pSprite);

    Object_2C* field_0_fire_hit_obj;
    Object_2C* field_4;
};

EXTERN_GLOBAL(Particle_8*, gParticle_8_6FD5E8);

using T_Particle_4C_Pool = Pool<Particle_4C, 500>;

EXTERN_GLOBAL(T_Particle_4C_Pool*, gParticle_4C_Pool_6FD5E4);
