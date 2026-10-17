#pragma once

// TODO: Replace CarDamageAreasFlags etc with this
namespace CarDeltaBitsEnum
{
enum
{
    kTopLeftDamage = 0, // (1 << 0)  = 0x00000001
    kTopRightDamage = 1, // (1 << 1)  = 0x00000002
    kBottomRightDamage = 2, // (1 << 2)  = 0x00000004
    kBottomLeftDamage = 3, // (1 << 3)  = 0x00000008
    kWindshieldDamage = 4, // (1 << 4)  = 0x00000010

    kBackRightBrakeLight = 5, // (1 << 5)  = 0x00000020
    kFrontRightHeadlight = 6, // (1 << 6)  = 0x00000040

    kBottomRightDoor1 = 7, // (1 << 7)  = 0x00000080
    kBottomRightDoor2 = 8, // (1 << 8)  = 0x00000100
    kBottomRightDoor3 = 9, // (1 << 9)  = 0x00000200
    kBottomRightDoor4 = 10, // (1 << 10) = 0x00000400

    kTopRightDoor1 = 11, // (1 << 11) = 0x00000800
    kTopRightDoor2 = 12, // (1 << 12) = 0x00001000
    kTopRightDoor3 = 13, // (1 << 13) = 0x00002000
    kTopRightDoor4 = 14, // (1 << 14) = 0x00004000

    kBottomLeftRoofLight = 15, // (1 << 15) = 0x00008000
    kBottomRightRoofLight = 16, // (1 << 16) = 0x00010000
    kTopLeftRoofLight = 17, // (1 << 17) = 0x00020000
    kTopRightRoofLight = 18, // (1 << 18) = 0x00040000

    kBit19 = 19, // (1 << 19) = 0x00080000
    kBit20 = 20, // (1 << 20) = 0x00100000
    kBit21 = 21, // (1 << 21) = 0x00200000

    kBackLeftBrakeLight = 22, // (1 << 22) = 0x00400000
    kFrontLeftHeadlight = 23, // (1 << 23) = 0x00800000

    kBottomLeftDoor1 = 24, // (1 << 24) = 0x01000000
    kBottomLeftDoor2 = 25, // (1 << 25) = 0x02000000
    kBottomLeftDoor3 = 26, // (1 << 26) = 0x04000000
    kBottomLeftDoor4 = 27, // (1 << 27) = 0x08000000

    kTopLeftDoor1 = 28, // (1 << 28) = 0x10000000
    kTopLeftDoor2 = 29, // (1 << 29) = 0x20000000
    kTopLeftDoor3 = 30, // (1 << 30) = 0x40000000
    kTopLeftDoor4 = 31 // (1 << 31) = 0x80000000
};
} // namespace CarDeltaBitsEnum

namespace CarManagerCarType
{
enum
{
    kNewName = 0,
    kNewName1 = 1,
    kNewName2 = 2,
    kNewName3 = 3,
    kAmbulance = 4,
    kFiretruck = 5,
    kPoliceCars = 6
};
// static_assert(sizeof(car_manager_car_type_enum) == 4);
} // namespace CarManagerCarType
namespace CarModel
{
enum
{
    kAlfa = 0,
    kAllard = 1,
    kAmdb4 = 2,
    kApc = 3,
    kBankVan = 4,
    kBmw = 5,
    kBoxcar = 6,
    kBoxtruck = 7,
    kBug = 8,
    kCar9 = 9,
    kBuick = 10,
    kBus = 11,
    kCopcar = 12,
    kDart = 13,
    kEdsel = 14,
    kCar15 = 15,
    kFiat = 16,
    kFiretruk = 17,
    kGraham = 18,
    kGt24640 = 19,
    kCar20 = 20,
    kGtruck = 21,
    kGunjeep = 22,
    kHotdog = 23,
    kHotdogD1 = 24,
    kHotdogD2 = 25,
    kHotdogD3 = 26,
    kIcecream = 27,
    kIsetlimo = 28,
    kIsetta = 29,
    kJeep = 30,
    kJeffrey = 31,
    kLimo = 32,
    kLimo2 = 33,
    kMedicar = 34,
    kMerc = 35,
    kMesser = 36,
    kMiura = 37,
    kMonster = 38,
    kMorgan = 39,
    kMorris = 40,
    kPickup = 41,
    kRtype = 42,
    kCar43 = 43,
    kSpider = 44,
    kSprite = 45,
    kStingray = 46,
    kStratos = 47,
    kStratosb = 48,
    kStripetb = 49,
    kStype = 50,
    kStypecab = 51,
    kSwatvan = 52,
    kT2000gt = 53,
    kTank = 54,
    kTanker = 55,
    kTaxi = 56,
    kTbird = 57,
    kTowtruck = 58,
    kTrain = 59,
    kTraincab = 60,
    kTrainfb = 61,
    kTranceam = 62,
    kTrukcab1 = 63,
    kTrukcab2 = 64,
    kTrukcont = 65,
    kTruktrns = 66,
    kTvvan = 67,
    kVan = 68,
    kVespa = 69,
    kVtype = 70,
    kWbtwin = 71,
    kWreck0 = 72,
    kWreck1 = 73,
    kWreck2 = 74,
    kWreck3 = 75,
    kWreck4 = 76,
    kWreck5 = 77,
    kWreck6 = 78,
    kWreck7 = 79,
    kWreck8 = 80,
    kWreck9 = 81,
    kXk120 = 82,
    kZcx5 = 83,
    kEdselfbi = 84,
    kHotdogD4 = 85,
    kKrsnabus = 86,
    kNone = 87
};
// static_assert(sizeof(car_model_enum) == 4);
} // namespace CarModel

namespace CarDoorLock
{
enum
{
    kNoLock = 0,
    kLocked = 1,
    kLockoutThief = 2,
    kUnlocked = 3,
    kLockedPermanently = 4,
    kLockoutPlayer = 5,
    kNone = 6,
};
} // namespace CarDoorLock

// Wanted level (star count / cop head), what Police_7B8 sends at each
namespace CopLevel
{
enum Level
{
    kNone = 0,
    kPolice1 = 1, // one police car
    kPolice2 = 2, // two police cars
    kPolice3 = 3, // roadblocks start
    kSwat = 4, // a SWAT crew is added
    kFbi = 5, // FBI crews
    kArmy = 6, // army
};
} // namespace CopLevel

namespace CopLevelPed
{
enum
{ /* The cop level stored in the PED */
    kCop0Stars = 0,
    kCop1Stars = 600,
    kCop2Stars = 1600,
    kCop3Stars = 3000,
    kCop4Stars = 5000,
    kCop5Stars = 8000,
    kCop6Stars = 12000
};
// static_assert(sizeof(cop_level_ped_enum) == 2);
} // namespace CopLevelPed

namespace GmpZoneType
{
enum
{
    kGeneralPurpose = 0,
    kNavigation = 1,
    kTrafficLight = 2,
    kUnused1 = 3,
    kUnused2 = 4,
    kArrowBlocker = 5,
    kRailwayStationPlatform = 6,
    kBusStopPavement = 7,
    kGeneralTrigger = 8,
    kUnused3 = 9,
    kInformation = 10,
    kRailwayStationEntryPoint = 11,
    kRailwayStationExitPoint = 12,
    kRailwayStopPoint = 13,
    kGang = 14,
    kLocalNavigation = 15,
    kRestart = 16,
    kUnused4 = 17,
    kUnused5 = 18,
    kUnsued6 = 19,
    kArrestRestart = 20
};
// static_assert(sizeof(gmp_zone_type_enum) == 4);
} // namespace GmpZoneType

namespace CrewType // KFC type
{
enum
{
    kParamedic = 1,
    kPolice = 3,
    kFbi = 4,
    kSwat = 5,
    kArmy = 6,
};
} // namespace CrewType

// Which kind of crew (or roadblock guard) is built: PoliceCrew_38::field_20_crew_kind and gCrewKind_6FEDB8
namespace CrewKind
{
enum
{
    kPolice = 1,
    kSwat = 2,
    kFbi = 3,
    kArmy = 4,
};
} // namespace CrewKind

namespace PaletteTypes
{
enum
{
    kWrongType = 0,
    kTiles = 1,
    kSprites = 2,
    kCarRemaps = 3,
    kPedRemaps = 4,
    kCodeObjRemaps = 5,
    kMapObjRemaps = 6,
    kUserRemaps = 7,
    kFontRemaps = 8,
};
// static_assert(sizeof(palette_types_enum) == 4);
} // namespace PaletteTypes

namespace PedBitStatus
{
enum
{
    kKPed0x00000001 = 0x00000001,
    kKPed0x00000002 = 0x00000002,
    kKPed0x00000004 = 0x00000004,
    kKPed0x00000008 = 0x00000008,
    kKPed0x00000010 = 0x00000010,
    kKPed0x00000020 = 0x00000020,
    kKPed0x00000040 = 0x00000040,
    kKPed0x00000080 = 0x00000080,
    kKPed0x00000100 = 0x00000100,
    kKPed0x00000200 = 0x00000200,
    kKPed0x00000400 = 0x00000400,
    kKPedHasWeapon = 0x00000800,
    kKPed0x00001000 = 0x00001000,
    kKPed0x00002000 = 0x00002000,
    kKPed0x00004000 = 0x00004000,
    kKPed0x00008000 = 0x00008000,
    kKPed0x00010000 = 0x00010000,
    kKPed0x00020000 = 0x00020000,
    kKPed0x00040000 = 0x00040000,
    kKPed0x00080000 = 0x00080000,
    kKPed0x00100000 = 0x00100000,
    kKPed0x00200000 = 0x00200000,
    kKPed0x00400000 = 0x00400000,
    kKPed0x00800000 = 0x00800000,
    kKPedInFlames = 0x01000000,
    kKPedInvisible = 0x02000000,
    kKPed0x04000000 = 0x04000000,
    kKPed0x08000000 = 0x08000000,
    kKPed0x10000000 = 0x10000000,
    kKPed0x20000000 = 0x20000000,
    kKPed0x40000000 = 0x40000000,
    kKPedElectroFingers = 0x40000000,
    kKPed0x80000000 = 0x80000000,
};
// static_assert(sizeof(ped_bit_status_enum) == 0x4);
} // namespace PedBitStatus

namespace PedMajorState
{
enum
{
    kPedMajorNewName = 0,
    kPedMajorWalking = 1,
    kPedMajorNewName1 = 2,
    kPedMajorNewName2 = 3,
    kPedMajorStaying = 4,
    kPedMajorDriving = 5,
    kPedMajorEnteringACar = 6,
    kPedMajorGettingOutACar = 7
};
// static_assert(sizeof(ped_major_state) == 0x1);
} // namespace PedMajorState

namespace PedOcupation
{
enum
{ /* The ocupation that a PED can have */
    kPlayer = 0,
    kEmpty = 1,
    kUnknown1 = 2,
    kDummy = 3,
    kUnknown2 = 4,
    kDriver = 5,
    kUnknown3 = 6,
    kTaxiCustomer = 7,
    kBusCustomer = 8,
    kTrainCustomer = 9,
    kRobbedDriver = 10, // victim of a steal
    kFleeingRobbedDriver = 11,
    kAngryArmedRobbedDriver = 12, // get the car back from the thief
    kVeryAngryArmedRobbedDriver = 13, // shoots at the thief
    kPsycho = 14,
    kMugger = 15,
    kCarThief = 16,
    kBankRobber = 17,
    kCriminal = 18,
    kArmedGangMember = 19,
    kUnknown11 = 20,
    kUnknown12 = 21,
    kElvis = 22,
    kParamedic = 23,
    kPolice = 24,
    kSwat = 25,
    kFbi = 26,
    kArmyArmy = 27,
    kGuard = 28,
    kWalkingGuard = 29,
    kUnknownCopOccu30 = 30,
    kUnknownCopOccu31 = 31,
    kGuardAgainstPlayer = 32,
    kCriminalType1 = 33,
    kCriminalType2 = 34,
    kSpecialGroupsMember = 35,
    kTankDriver = 36,
    kRoadblockCop = 37,
    kFireman = 38,
    kRoadBlockTankMan = 39,
    kMadMugger = 40, // npcs spawned when MAKE_ALL_CHARS_MUGGERS is active
    kDrone = 41,
    kGangDriver = 42,
    kStandStillBloke = 43,
    kElvisLeader = 44,
    kRefugees = 45,
    kAnyLawEnforcement = 46,
    kAnyEmergencyServiceMan = 47,
    kAnyGangMember = 48,
    kAnyElvis = 49,
    kScaredDriver = 50, // scared by shooting at his car
    kNoOccupation = 51,
};
// static_assert(sizeof(ped_ocupation_enum) == 0x4);
} // namespace PedOcupation

namespace PedRemap
{
enum
{
    kPedRemapBluePolice = 0,
    kPedRemapGreenPolice = 1,
    kPedRemapRedPolice = 2,
    kPedRemapYellowPolice = 3,
    kPedRemapArmy = 4,
    kPedRemapRedHeadRedneck = 5,
    kPedRemapBlondHeadRedneck = 6,
    kPedRemapYellowScientist = 7,
    kPedRemapZaibatsu = 8,
    kPedRemapKristna = 9,
    kPedRemapRussian = 10,
    kPedRemapLonny = 11,
    kPedRemapElvis = 12,
    kPedRemapYakuza = 13,
    kPedRemapFireman = 14,
    kPedRemapGreenShorts = 15,
    kPedRemapMedic = 16,
    kPedRemapMugger = 17,
    kPedRemapBlueDummy = 18,
    kPedRemapLightBlueDummy = 19,
    kPedRemapTshirtAndShortsDummy = 20,
    kPedRemapShortSleeve = 21,
    kPedRemapPrisonUniform = 22,
    kPedRemapHulk1Normal = 23,
    kPedRemapHulk2Green = 24,
    kPedRemapPlayer = 25,
    kPedRemapNakedDummy = 26,
};
// static_assert(sizeof(ped_remap_enum) == 0x1);
} // namespace PedRemap

namespace PedState1Alias
{
enum
{
    kPedWalking = 0,
    kUnused1 = 1,
    kPedGoingToCar = 2,
    kPedEnteringACar = 3,
    kPedGettingOutACar = 4,
    kUnused2 = 5,
    kUnused3 = 6,
    kPedStaying = 7,
    kPedFallOnGround = 8,
    kPedWasted = 9,
    kPedDriving = 10,
};
// static_assert(sizeof(ped_state1_enum) == 0x4);
} // namespace PedState1Alias

namespace PedState2
{
enum
{
    kPed2Walking = 0,
    kUnknown1 = 1,
    kUnknown2 = 2,
    kUnknown3 = 3,
    kPed2FollowingACar = 4,
    kUnknown5 = 5,
    kPed2EnteringACar = 6,
    kPed2GettingOutACar = 7,
    kUnknown8 = 8,
    kUnknown9 = 9,
    kPed2Driving = 10,
    kPed2Staying = 14,
    kUnknown15 = 15,
    kUnknown16 = 16,
    kUnknown17 = 17,
    kUnknown18 = 18,
    kFalling = 19,
    kSinking = 20,
    kUnknown21 = 21,
    kLyingOnFloor = 22,
    kUnknown23 = 23,
    kUnknown24 = 24,
    kUnknown25 = 25,
    kUnknown26 = 26,
    kElectrocuted = 27,
    kUnknown28 = 28,
    kUnknown29 = 29,
    kUnknown30 = 30,
};
// static_assert(sizeof(ped_state_2) == 0x4);
} // namespace PedState2

namespace CharB4State
{
enum
{
    kUnknown0 = 0,
    kUnknown1 = 1,
    kUnknown2 = 2,
    kUnknown3 = 3,
    kUnknown4 = 4,
    kUnknown5 = 5,
    kUnknown6 = 6,
    kUnknown7 = 7,
    kUnknown8 = 8,
    kUnknown9 = 9,
    kUnknown10 = 10,
    kUnknown11 = 11,
    kUnknown12 = 12,
    kUnknown13 = 13,
    kUnknown14 = 14,
    kJumping = 15,
    kCollidingWithCar = 27,
    kSmoking = 35,
    kInteractingCarDoor = 36,
};
} // namespace CharB4State

namespace CharAnimState
{
enum
{
    kUnknown0 = 0,
    kUnknown1 = 1,
    kUnknown2 = 2,
    kUnknown3 = 3,
    kUnknown4 = 4,
    kJumping = 5,
    kEnteringCar = 6,
    kExitingCar = 7,
    kGettingPulledOutOfCar = 8,
    kUnknown9 = 9,
    kUnknown10 = 10,
    kNormalFall = 11,
    kLethalFall = 12,
    kUnknown13 = 13,
    kUnknown14 = 14,
};
} // namespace CharAnimState

namespace SpecSurfaceType
{
enum
{
    kSpecGrass = 2,
    kSpecRoadSpecial = 3,
    kSpecWater = 4,
    kSpecElectrified = 5,
    kSpecElectrifiedPlatform = 6,
    kSpecWoodFloor = 7,
    kSpecMetalFloor = 8,
    kSpecMetalWall = 9,
    kSpecGrassDirtWall = 10,
};
// static_assert(sizeof(spec_surface_type_enum) == 4);
} // namespace SpecSurfaceType

namespace CarSurfaceType
{
enum
{
    kFlatSurface = 0,
    kSlopeNorthwards = 1,
    kSlopeSouthwards = 2,
    kSlopeWestwards = 3,
    kSlopeEastwards = 4,
    kAirSurface = 6,
    kUnknownSurface7 = 7,
    kWaterSurface = 8,
    kUnknownSurface9 = 9,
};
} // namespace CarSurfaceType

namespace SpriteTypes
{
enum
{
    kUnknown = 0, // At least in the function sprite_FUN_004b9aa0, but don't know what represents.
    kObject = 1, // has an Object_2C like code_obj1_4 and map_obj_5
    kCar = 2,
    kPed = 3,
    kCodeObj1 = 4,
    kMapObj = 5,
    kUser = 6,
    kFont = 7,
    kCodeObj2 = 8,
};
// static_assert(sizeof(sprite_types_enum) == 4);
} // namespace SpriteTypes

namespace WeaponType
{
enum
{
    kPistol = 0,
    kSmg = 1,
    kRocket = 2,
    kShocker = 3,
    kMolotov = 4,
    kGrenade = 5,
    kShotgun = 6,
    kElectroBatton = 7,
    kFlamethrower = 8,
    kSilenceSmg = 9,
    kDualPistol = 10,
    kWeapon11 = 11,
    kWeapon12 = 12,
    kWeapon13 = 13,
    kWeapon14 = 14,
    kCarBomb = 15,
    kOilStain = 16,
    kCarMines = 17,
    kCarSmg = 18,
    kTankMainGun = 19,
    kFireTruckGun = 20,
    kFireTruckFlamethrower = 21,
    kArmyGunJeep = 22,
    kWeapon0x17 = 23,
};
// static_assert(sizeof(weapon_type) == 0x4);
} // namespace WeaponType

namespace Objects
{
enum
{
    kObject0 = 0,
    kBinLid = 1,
    kBollard = 2,
    kCone = 3,
    kBoxes = 4,
    kBlaster = 5,
    kRubbish = 6,
    kBin = 7,
    kAnimatingOil = 8,
    kOil = 9,
    kMine = 10,
    kBush = 11,
    kCrate = 12,
    kFooty = 13,
    kHardbox = 14,
    kNewsdis = 15,
    kOildrum = 16,
    kTyre = 17,
    kHydrantLid = 18,
    kHydrant = 19,
    kHydrantUnlid = 20,
    kRoadblock = 21,
    kBench = 22,
    kPackage = 23,
    kObject24 = 24,
    kTower = 25,
    kObject26 = 26,
    kObject27 = 27,
    kObject28 = 28,
    kObject29 = 29,
    kObject30 = 30,
    kObject31 = 31,
    kObject32 = 32,
    kObject33 = 33,
    kObject34 = 34,
    kObject35 = 35,
    kObject36 = 36,
    kObject37 = 37,
    kObject38 = 38,
    kObject39 = 39,
    kObject40 = 40,
    kExplodeMedium = 41,
    kObject42 = 42,
    kObject43 = 43,
    kObject44 = 44,
    kObject45 = 45,
    kObject46 = 46,
    kMovingCollect00 = 96,
    kMovingCollect01 = 97,
    kMovingCollect02 = 98,
    kMovingCollect03 = 99,
    kMovingCollect04 = 100,
    kMovingCollect05 = 101,
    kMovingCollect06 = 102,
    kMovingCollect07 = 103,
    kMovingCollect08 = 104,
    kMovingCollect09 = 105,
    kMovingCollect10 = 106,
    kMovingCollect11 = 107,
    kMovingCollect12 = 108,
    kMovingCollect13 = 109,
    kPedPushed = 110,
    kMovingCollect15 = 111,
    kMovingCollect16 = 112,
    kExplosion = 113,
    kWaterCannon = 114,
    kMovingCollect19 = 115,
    kMovingCollect20 = 116,
    kMovingCollect21 = 117,
    kMovingCollect22 = 118,
    kMovingCollect23 = 119,
    kRoadSkidmark = 120,
    kMovingCollect25 = 121,
    kPedestrianCrossingMarker = 122,
    kMovingCollect27 = 123,
    kMovingCollect28 = 124,
    kMovingCollect29 = 125,
    kMovingCollect30 = 126,
    kExplosionFragments = 127,
    kRocketBullet = 128,
    kBusStopMarker = 129,
    kCarShop130 = 130,
    kCarShopBusy = 131,
    kMovingCollect36 = 132,
    kMovingCollect37 = 133,
    kCraneUnknown = 134,
    kCraneRotor = 135,
    kCraneHook = 136,
    kCraneBase = 137,
    kMovingMolotov = 138,
    kConveyor = 139,
    kCraneCounterweight = 140,
    kDestructor = 141,
    kCrusherPiston = 142,
    kCrusherCentralSpot = 143,
    kObject144 = 144,
    kParticleSystem = 145,
    kFirejet = 146,
    kObject147 = 147,
    kSmallBrownSkid = 148,
    kSmallGreySkid = 149,
    kCrusherPistonBase = 150,
    kMediumBrownSkid = 151,
    kMediumGreySkid = 152,
    kMediumRedSkid = 153,
    kFlamethrowerFire = 154,
    kCarStop = 155,
    kBigWhiteSkid = 156,
    kMediumWhiteSkid = 157,
    kSmallWhiteSkid = 158,
    kObject159 = 159,
    kRocket = 160,
    kSavepoint = 161,
    kCarShop162 = 162,
    kBusyCarShop = 163,
    kBluePhone = 164,
    kMaybeTrainHeadlight = 165,
    kDiagonalWallCollisionObj = 166,
    kMaybeDoorTrigger = 167,
    kDoorUnknown = 169,
    kTrafficLightHull = 170,
    kRedPhone = 177,
    kYellowPhone = 179,
    kGreenPhone = 181,
    kGrenadeObj = 183,
    kGrenadeObjOnGround = 184,
    kHugeRedSkid = 189,
    kHugeWhiteSkid = 190,
    kHugeBrownSkid = 191,
    kShotgunBullet = 192,
    kTanktop = 193,
    kFireHitting = 194,
    kObject195 = 195,
    kAnimatingRubbish = 196,
    kFire = 197, // like the one in peds
    kMaybeBulletOnFire = 198,
    kObject199 = 199,
    kRemote = 200,

    kMachineGunWeapon = 201,
    kMolotovWeapon = 204,
    kShotgunWeapon = 206,
    kFlamethrowerWeapon = 208,

    kHealthPowerup = 230,
    kArmorPowerup = 231,
    kGetOutJailPowerup = 232,
    kInvulnerabilityPowerup = 234,
    kInvisibilityPowerup = 239,

    kShopCarSmg = 250,
    kShopCarOilStain = 251,
    kShopCarMines = 252,

    kMachineGunBullet = 254,
    kTrafficLightLighting = 255,
    kPedCrossingTrigger = 258,
    kPistolBullet = 265,
    kSecretToken = 266,

    kElectrobatonBullet = 277,
    kSoundObjectType1 = 278,
    kSoundObjectType2 = 279,
    kPowerGenerator = 281,
    kDestroyedPowergen = 282,
    kInvisiblePowergen = 283,
    kPowergenLight = 284,
    kDestroyedInvisiblePowergen = 285,
    kKillFrenzyIcon = 286,
    kLooniesIcon = 287,
    kYakuzaIcon = 288,
    kZaibatsuIcon = 289,
    kRedneckIcon = 290,
    kScientistsIcon = 291,
    kKrishnaIcon = 292,
    kRussianMafiaIcon = 293,
    kTunnelBlocker = 294,
};
} // namespace Objects

namespace ThreatReaction
{
enum
{
    kNoReaction = 0,
    kReactAsEmergency = 1,
    kReactAsNormal = 2,
    kRunAway = 3,
};
} // namespace ThreatReaction

namespace ObjectiveStatus
{
enum
{
    kNotFinished = 0,
    kPassed = 1,
    kFailed = 2,
};
} // namespace ObjectiveStatus

namespace Objectives
{
enum
{
    kNoObj = 0,
    kFleeOnFootTillSafe = 1,
    kFleeCharOnFootTillSafe = 2,
    kFleeCharOnFootAlways = 3,
    kFleeCharAnyMeansTillSafe = 4,
    kFleeCharAnyMeansAlways = 5,
    kFleeCharAlwaysOnceCarStopped = 6,
    kObjective7 = 7,
    kObjective8 = 8,
    kObjective9 = 9,
    kObjective10 = 10,
    kObjective11 = 11,
    kGotoAreaOnFoot = 12,
    kGotoAreaAnyMeans = 13,
    kGotoAreaInCar = 14,
    kObjective15 = 15,
    kGotoCharOnFoot = 16,
    kObjective17 = 17,
    kObjective18 = 18,
    kKillCharAnyMeans = 19,
    kKillCharOnFoot = 20,
    kKillCar = 21,
    kKillFrenzy = 22,
    kPunchChar = 23,
    kGuardSpot = 24,
    kGuardArea = 25,
    kWaitOnFoot = 26,
    kWaitInCar = 27,
    kObjective28 = 28,
    kObjective29 = 29,
    kObjective30 = 30,
    kTimeWaitedInCar = 31,
    kObjective32 = 32,
    kObjective33 = 33,
    kObjective34 = 34,
    kEnterCarAsDriver = 35,
    kLeaveCar = 36,
    kEnterTrain = 37,
    kLeaveTrain = 38,
    kObjective39 = 39,
    kObjective40 = 40,
    kObjective41 = 41,
    kPatrolOnFoot = 42,
    kObjective43 = 43,
    kObjective44 = 44,
    kObjective45 = 45,
    kObjective46 = 46,
    kObjective47 = 47,
    kObjective48 = 48,
    kObjective49 = 49,
    kObjective50 = 50,
    kObjective51 = 51,
    kObjective52 = 52,
    kObjective53 = 53,
    kObjective54 = 54,
    kFollowCarInCar = 55,
    kFollowCarOnFootWithOffset = 56,
    kFireAtObjectFromVehicle = 57,
    kDestroyObject = 58,
    kDestroyCar = 59,
    kTurretPutOutCarFire = 60,
    kObjective61 = 61,
};
} // namespace Objectives

namespace ThreatSearch
{
enum
{
    kNoThreats = 0,
    kLineOfSight = 1,
    kArea = 2,
    kAreaPlayerThreatOnly = 3,
    kLineOfSightPlayerThreatOnly = 4,
    kAreaPlayerOnly = 5,
    kLineOfSightPlayerOnly = 6,
};
} // namespace ThreatSearch

namespace PedType
{
enum
{
    kUnknown0 = 0,
    kUnknown1 = 1,
    kPlayer = 2,
    kDummy = 3,
    kSpecialPed = 4,
    kScriptCreated = 5,
    kDummyWithOccupation = 6,
};
} // namespace PedType

// Keeps the number of distinct names in headers MapRenderer.cpp includes unchanged after merging ped_type_enum into
// ped_type; it flips Draw3SidedDiagonalDownRight_4EF520 / Draw4SidedDiagonalUpLeft_4EF880 otherwise (see car_flags.hpp
// and docs/matching_quirks.md). Do not remove without running the whole build.
enum
{
    kPedTypeNameCount0,
    kPedTypeNameCount1,
    kPedTypeNameCount2,
    kPedTypeNameCount3,
    kPedTypeNameCount4
};

namespace PedState1
{
enum
{
    kWalking = 0,
    kFleeOrRunning = 1,
    kFindindPath = 2,
    kEnteringCar = 3,
    kExitingCar = 4,
    kUnknown5 = 5,
    kUnknown6 = 6,
    kStandingStill = 7,
    kImmobilized = 8, // ped lying on ground, or getting repeled by blast waves, or getting pushed, or on air, or sinking
    kDead = 9,
    kInCar = 10,
};
} // namespace PedState1

namespace CarDamageAreas
{
enum
{
    kFrontLeft = 0x0,
    kFrontRight = 0x1,
    kBackLeft = 0x2,
    kBackRight = 0x3,
    kWindow = 0x4,
};
} // namespace CarDamageAreas

namespace CarDamageAreasFlags
{
enum
{
    kFlagsBackRight = 0x1,
    kFlagsBackLeft = 0x2,
    kFlagsFrontLeft = 0x4,
    kFlagsFrontRight = 0x8,
    kFlagsWindow = 0x10,
};
} // namespace CarDamageAreasFlags

namespace Gta2Error
{
enum Gta2Error
{
    kDMAVideoInitFail = 2U,
    kSetWindowModeFail = 5U,
    kGetValidHDCFail = 6U,
    kSetWindowPosFail = 7U,
    kDirectInputInitFail = 8U,
    kDirectInputSetDataFormatFail = 9U,
    kDirectInputSetCooperativeLevelFail = 10U,
    kFtellError = 13U,
    kFseekError = 14U,
    kFileReadFailure = 15U,
    kFreeloaderEpisodeUnknown = 16U,
    kFileCloseError = 17U,
    kFileBiggerThanBuffer = 18U,
    kWritingZeroBytesToFile = 19U,
    kFileWriteFailure = 20U,
    kFileReadingNotOpenedFile = 21U,
    kUngetcFailrue = 24U,
    kFileInvalidInteger = 25U,
    kFileEndOfFileInTextString = 26U,
    kTextStringTooLong = 27U,
    kZeroBytesAlloc = 28U,
    kDirectInputNoObject = 30U,
    kDirectInputCreateFail = 31U,
    kOutOfMemoryNewOperator = 32U,
    kFileOverflow = 34U,
    kDataNotLoaded = 35U,
    kInvalidStringInParameterList = 36U,
    kRepeatedFileInParameterList = 37U,
    kInvalidZoneData = 39U,
    kInvalidMapObjectData = 40U,
    kInvalidLoadedObjectInfoData = 41U,
    kCloseRegistryKeyFail = 42U,
    kCreateRegistryKeyFail = 43U,
    kNoPlayers = 44U,
    kClassNotAllocated = 45U,
    kSetRegistryValueFail = 46U,
    kLoopInTree = 47U,
    kDisplayTreeOutOfSpace = 48U,
    kDivideByZero = 49U,
    kFileWriteByteFail = 50U,
    kInvalidMapLightData = 51U,
    kUsingUninitialisedSprite = 53U,
    kInvalidCorners = 54U,
    kMissionProcessAllocMemFail = 55U,
    kMissionProcessOutOfSlots = 56U,
    kCheckWallsSlopeInvalidSlope = 59U,
    kNoPlayerPointer = 60U,
    kCharacterHasNoPedPointer1 = 61U,
    kCharacterDefaultCase = 62U,
    kInvalidObjectiveCoordinates = 64U,
    kScriptfileNoLEVELSTARTDeclaration = 65U,
    kScriptfileTooManyPLAYERPEDDeclarations = 66U,
    kMapChangeOverflowBlocks = 67U,
    kMapChangeOverflowColumns = 68U,
    kScriptfileFailedToCreatePed = 69U,
    kMissionStackAllocFail = 70U,
    kMissionStackPushNullItem = 71U,
    kSecurityFail = 72U,
    kMissStackItemGrabNoStackSpaceLeftFail = 73U,
    kRemoveCharFromCharlistFail = 74U,
    kInvalidDoor = 75U,
    kScriptfileCreateCarFail = 76U,
    kObjectHasDataWhenItShouldnt = 77U,
    kCharacterHasNoCarObjectivePointer1 = 79U,
    kCharacterHasNoCarObjectivePointer2 = 80U,
    kCharacterHasNoPedPointer2 = 81U,
    kGroupMemberNotRemoved = 82U,
    kTooManyGroupMembers = 83U,
    kGroupCreateTooManyGroupsInUse = 84U,
    kPedDefaultCase = 85U,
    kPedSpriteZIs0 = 88U,
    kPedsCarInfoIsNULL = 89U,
    kSidTooManyReferences = 90U,
    kSidNegativeReferences = 91U,
    kNoFreeTiles = 92U,
    kInvalidFileType = 93U,
    kInvalidFileVersion = 94U,
    kCharacterNoFootOrCarPointer = 95U,
    kCreateMedicalUnitFail = 96U,
    kCharacterHasNoInCarPointer = 97U,
    kTooManyTrafficLights = 98U,
    kTooManyInfoAndNavigationZonesInMap = 99U,
    kInvalidZoneForInfoSetting = 100U,
    kFailedToCreateCarInRecycle = 101U,
    kEmptyStringPassedToFunction = 102U,
    kFirstPlayerIsNULL = 103U,
    kNextPlayerIsNULL = 104U,
    kNullBlockPassedToCheckForArrowsInRoutefinderCode = 105U,
    kTooManyJunctionsInMapForRoutefinder = 106U,
    kCarTryingSuicideDidntDealWithDriver = 107U,
    kAIObjectiveRequiredByCharNoLongerActive = 108U,
    kNoMoreSpaceInSimpTextDisplayBuffer = 109U,
    kTooManyScoreTextures = 120U,
    kTryingToGetUnitOfZeroVector = 121U,
    kAssertionFail = 123U,
    kTryingToExpandAnEmptyNodeInRouterfinder = 124U,
    kNoValidBackcarForAnCarRelatedStruct = 125U,
    kGta2ManagerLaunchError = 126U,
    kMessageTextTooLongToWrap = 127U,
    kTooManyTimers = 128U,
    kTooManyMessages = 129U,
    kTooManyArrows = 130U,
    kNoRestartZone = 131U,
    kMapNotCompressed = 132U,
    kInvalidSmybolDataInSymboltableBuffer = 133U,
    kInvalidMissionScriptFileSpecifiedInBob = 134U,
    kPointingArrowAtANoLongerValidCharacter = 136U,
    kPointingArrowAtANoLongerValidCar = 137U,
    kPointingArrowAtANoLongerValidObject = 138U,
    kCarRoutefindNoRoutfinderDataInMap = 139U,
    kIllegalTrainStationNoPlatformZone = 143U,
    kNoMoreTrainSpace = 144U,
    kBaseChangeLogOverflow = 145U,
    kGswTrainMovingAcrossSlope = 146U,
    kGswTrainUnknownDirection = 147U,
    kGswTrainUnknownSlope = 148U,
    kInvalidCarRating = 149U,
    kKeyboardTypeNotHandled = 150U,
    kKeyboardTypeFileOpenError = 151U,
    kSeqFileNotFound = 152U,
    kMultipleSeqFilesFound = 153U,
    kSeqFileOpenError = 154U,
    kTooManyMainBlocks = 155U,
    kMainBlockMustPrecedeBonus = 156U,
    kTooManyBonusBlocks = 157U,
    kInvalidBlockType = 158U,
    kInvalidFirstLineCharacter = 159U,
    kLineInterruptedByNewline = 160U,
    kLabelTooLong = 161U,
    kUndefinedLabel = 162U,
    kLineDataTooLong = 163U,
    kUnexpectedLabel = 164U,
    kNoValidMenuOptions = 165U,
    kGmpFilenameMismatch = 166U,
    kStyFilenameMismatch = 167U,
    kScrFilenameMismatch = 168U,
    kBinkBufferOpenError = 169U,
    kBinkOpenError = 170U,
    kCreditsLineTooLong = 183U,
    kTooManyCreditsLines = 184U,
    kInvalidLevelAdvancement = 185U,
    kLevelNotOpened = 186U,
    kInvalidBackgroundImageSize = 187U,
    kInvalidCreditTextColor = 188U,
    kInvalidMultiplayerGameType = 189U,
    kUnableToHandleAttachedPed = 171U,
    kEmptyReplayFile = 172U,
    kReplaySyncError = 173U,
    kTargaMemoryAllocationError = 174U,
    kTargaFileInvalidImage = 175U,
    kTargaFileInvalidFormat = 176U,
    kTargaFileInvalidBitsPerPixel = 177U,
    kTargaFileHasPalette = 178U,
    kTargaFileInvalidOrigin = 179U,
    kTargaFileZeroWidthOrHeight = 180U,
    kTargaFileIdReadError = 181U,
    kTargaFileImageReadError = 182U,
    kInvalidCarModel = 1001U,
    kMultipleSpriteDeltas = 1002U,
    kTooManySpritesWithDeltas = 1003U,
    kInvalidAbsoluteSpriteNumber = 1004U,
    kTooManySprites = 1005U,
    kInvalidCase = 1006U,
    kInvalidAbsoluteVirtualPaletteNumber = 1007U,
    kInvalidFontNumber = 1008U,
    kInvalidTileNumber = 1009U,
    kInvalidNewTileNumber = 1010U,
    kDllLoadError = 1011U,
    kInvalidPhysicalPaletteNumber = 1012U,
    kZoneNotFoundError = 1013U,
    kInvalidCarModelGearSpeeds = 1014U,
    kTooManyZones = 1016U,
    kInvalidZoneNumber = 1017U,
    kInvalidMapObjectNumber = 1018U,
    kFileTooLarge = 1022U,
    kOutOfMemory = 1029U,
    kInvalidChunkSize = 1033U,
    kInvalidParameterListSwitch = 1034U,
    kInvalidLineInfo = 1073U,
    kReplayFileTooLarge = 1035U,
    kInvalidVideomode = 1036U,
    kFullscreenVideomodeSetError = 1037U,
    kTooManyLoadedObjectInfos = 1020U,
    kTooManyPlayers = 1023U,
    kInvalidPlayerNumber = 1024U,
    kInvalidGridWidth = 1025U,
    kInvalidGridHeight = 1026U,
    kInvalidZCoordinate = 1027U,
    kUnprocessedEvent = 1028U,
    kListTypeOutOfSpace = 1040U,
    kNoCollisionBoxInSprite = 1039U,
    kInvalidCollisionBoxInSprite = 1021U,
    kInvalidObjectModelNumber = 1041U,
    kObjectModelNumberAlreadyAdded = 1042U,
    kNoSpaceForObjectModelNumber = 1043U,
    kCheckWallsSlopeSizeTooBig = 1044U,
    kFailedToCreateObjectInScriptfile = 1045U,
    kAttemptToKillDeadMissionThread = 1046U,
    kTooManyCarDoors = 1047U,
    kReleasingNullPtrInReleasePassive = 1048U,
    kTryingToPacifyNullListElement = 1049U,
    kObjectBackPointerMissingInSprite = 1050U,
    kNoRigidBodyInCar = 1051U,
    kNoFreeSidForCharacter = 1052U,
    kInvalidDoorInfoNumber = 1053U,
    kIllegalAngleValue = 1054U,
    kInvalidTileTexture = 1055U,
    kInvalidCacheNumber = 1056U,
    kInvalidScoreTextureNumber = 1068U,
    kInvalidZoneInformation = 1019U,
    kCounterOutOfBoundsInternalCount = 1057U,
    kOutOfRangeRouteId = 1058U,
    kInvalidScorePosition = 1066U,
    kInvalidPlayerId = 1067U,
    kTooManyCharactersInScore = 1069U,
    kInvalidTextFile = 1071U,
    kStringStartsWithEndOfLine = 1072U,
    kCarInCollisionWithWallAtSafePosition = 1076U,
    kObjectInCollisionWithWallAtSafePosition = 1080U,
    kIllegalDeviceCode = 1077U,
    kActivatingNullPtrInActivate = 1078U,
    kActivatingItemTwice = 1079U,
    kIllegalTurntableIndex = 1081U,
    kTooManyYRoadSegments = 1082U,
    kTooManyXRoadSegments = 1083U,
    kInvalidTimerHandle = 1084U,
    kInvalidArrowHandle = 1085U,
    kUnallocatedArrowHandle = 1086U,
    kCarAlreadyDoingRoutefind = 1090U,
    kCounterCheckFailed = 1137U,
    kInvalidCarModelStyleData = 1107U,
    kReleasingTwiceInReleasePassive = 1138U,
    kReleasingTwiceInRelease = 1139U,
    kTooManySymbols = 1096U,
    kFailedToCreateCharacterInScript = 1098U,
    kFailedToFindUidInScript = 1099U,
    kIllegalZoneIdInSymbolTable = 1100U,
    kFailedToCreateArrowInScript = 1101U,
    kMultipleDeltaEntries = 1002U,
    kUnableToLoadDLL = 1011U,
    kZoneNotFound = 1013U,
    kInvalidGearSpeedsInGciFile = 1014U,
    kInvalidSwitchInParameterList = 1034U,
    kInvalidLine = 1073U,
    kInvalidVideomodeToSearch = 1036U,
    kFailedToSetFullscreenVideomode = 1037U,
    kEventLeftUnprocessed = 1028U,
    kOutOfSpaceForListType = 1040U,
    kCollisionBoxInvalidInSprite = 1021U,
    kAttemptToKillAlreadyDeadMissionThread = 1046U,
    kCarModelHasTooManyDoors = 1047U,
    kIllegalValueForAnAngle = 1054U,
    kSettingInvalidZoneInformation = 1019U,
    kCounterHasOutOfBoundsInternalCount = 1057U,
    kInvalidPlayerID = 1067U,
    kIllegalDeviceCodeForThisMachine = 1077U,
    kIllegalIndexIntoTurntable = 1081U,
    kCarModelHasInvalidStyleData = 1107U,
    kAssertionFailure = 1095U,
    kLineAccessingCarThatHasntBeenCreated = 1102U,
    kLineAccessingCharacterThatHasntBeenCreated = 1103U,
    kLineAccessingObjectThatHasntBeenCreated = 1104U,
    kLineAccessingCounterThatHasntBeenCreated = 1110U,
    kLineAccessingCharacterWithNoGroup = 1111U,
    kLineAccessingTriggerThatHasntBeenCreated = 1106U,
    kLineAccessingCraneThatHasntBeenCreated = 1108U,
    kLineOrderingDriverOutOfCarButCharIsNotDriver = 1109U,
    kLineAccessingArrowThatHasntBeenCreated = 1112U,
    kAttemptingToDoSetGangInfoOnInvalidGangName = 1113U,
    kFailedToCreateCraneInScriptLine = 1114U,
    kFailedToCreateConveyorInScriptLine = 1115U,
    kFailedToCreateGeneratorInScriptLine = 1116U,
    kFailedToCreateDestructorInScriptLine = 1117U,
    kAttemptingToPos32ArrowAtLogicalItem = 1118U,
    kLineAccessingDoorThatHasntBeenCreated = 1119U,
    kFailedToCreateCrusherInScriptLine = 1120U,
    kLineAccessingConveyorThatHasntBeenCreated = 1121U,
    kLineAccessingGeneratorThatHasntBeenCreated = 1123U,
    kInvalidSpecialTileNumber = 1124U,
    kRepeatedSpecialTileNumber = 1125U,
    kLineAccessingDestructorThatHasntBeenCreated = 1122U,
    kCheckingPhoneInMissionscriptWithoutExecutingAnswerPhone = 1126U,
    kMapHasTooManyColumns = 1127U,
    kMapHasTooManyBlocks = 1129U,
    kStyleFileContainsUnexpectedNumberOfFonts = 1128U,
    kAttemptingIsItemOnscreenOnIllegalType = 1130U,
    kUnableToCreateCarToPlaceOnTopOfTrailerInScript = 1131U,
    kUnableToFindZonenameToSetupInfoForInScript = 1132U,
    kUnableToFindZonenameInSetGangInfoInScript = 1133U,
    kUnableToFindZonenameInSetStationInfoInScript = 1134U,
    kUnableToFindPlatformInSetStationInfoInScript = 1135U,
    kUnableToFindTrainstationAssociatedWithPlatformInSetStationInfoInScript = 1136U,
    kUnableToFindGangMatchingNameInSetGangRespect = 1140U,
    kUnableToFindGangMatchingNameInSetGangRespect2 = 1141U,
    kUnableToFindGangnameInAlterGangRespect = 1142U,
    kTryingToChangeGangRespectOnNonPlayerControlledCharacter = 1143U,
    kUnableToFindGangMatchingNameInCheckGangRespect = 1144U,
    kUnableToFindZoneMatchingNameInIsCharInGangzone = 1145U,
    kUnableToFindZoneMatchingNameInIsCharInZone = 1146U,
    kUnableToFindGangMatchingNameInAddCharToGang = 1147U,
    kMissionScriptIsReferringToTextIdThatDoesNotExist = 1148U,
    kErrorWith1Param = 1400U,
    kInvalidRelativeSpriteNumber = 2001U,
    kInvalidRelativeVirtualPaletteNumber = 2002U,
    kInvalidRelativeFontSpriteNumber = 2003U,
    kIncorrectAspectRatioInScreenMode = 2004U,
    kListTypeCorruptedAtGrab = 2024U,
    kListTypeCorruptedAtRelease = 2025U,
    kListTypeCorruptedAtTraverse = 2026U,
    kListTypeCorruptedAtActivate = 2030U,
    kListTypeCorruptedAtPacify = 2031U,
    kCantDeleteElementFromList = 2005U,
    kInvalidZsInSprite = 2006U,
    kDoorTooCloseInCarModel = 2007U,
    kInvalidSpriteTypeAndNumber = 2008U,
    kTrafficLightsTooCloseToEdgeOfWorld = 2009U,
    kInvalidFrameInObjectModel = 2010U,
    kIllegalCoordinate = 2011U,
    kIllegalGoodBadRatioForCarModelChoice = 2012U,
    kModelRepeatedInFile = 2019U,
    kBinmakeError = 2014U,
    kInvalidSize = 2018U,
    kCarInCollisionWithSpriteAtSafePosition = 2015U,
    kObjectInCollisionWithSpriteAtSafePosition = 2016U,
    kInvalidCraneTarget = 2017U,
    kTooManyDoorInfosDeclaredInScript = 2020U,
    kIllegalTrainStationNoEntryZone = 2021U,
    kIllegalTrainStationNoExitZone = 2022U,
    kIllegalTrainStationNoStopZone = 2023U,
    kErrorWith2Params = 2401U,
    kIncorrectReplayVersion = 2027U,
    kSyncErrorRandom = 2028U,
    kSyncErrorMission = 2029U,
    kVideomodeNotAvailable = 3003U,
    kFailedToSetVideomode = 3004U,
    kCantDeleteSpriteFromCollisionList = 3005U,
    kAddingSpriteToCollisionListTwice = 3006U,
    kInvalidMapAccess = 3007U,
    kInvalidMapChangeAccess = 3008U,
    kInvalidPaletteTypeForSpriteTypeAndNumber = 3009U,
    kInvalidSpriteTypeAndNumberRemap = 3010U,
    kModelCountMismatchInStyleFile = 3013U,
    kInvalidBlockInTrainCode = 3011U,
    kTrainOnBlockWithIncorrectArrow = 3012U,
    kAttemptingToDisableRoadAtIllegalCoordinates = 3014U,
    kAttemptingToEnableRoadAtIllegalCoordinates = 3015U,
    kIllegalBlockForTrainCreation = 3016U,
    kFailedToCreateCarriage = 3017U,
    kErrorInSetupOfParkCommand = 3018U,
    kInvalidXCoordinateInSpriteType = 4001U,
    kInvalidYCoordinateInSpriteType = 4002U,
    kInvalidZCoordinateInSpriteType = 4003U,
    kPlacingGangPhoneWhenNoGang = 4007U,
    kIllegalCoordinate3D = 4004U,
    kInvalidDiagonalBlock = 4008U,
    kNoDirectionArrowFoundInBlockWithSlope = 4005U,
    kDirectXError = 9011U,
    kTooManyPhonesForGang = 8001U,
    kTooManyLinesInBriefing = 7004U,
    kCouldntFindAttractModeFile = 7006U,
    kTextIdentifierNotFound = 7001U,
    kBriefingTooLong = 7002U,
    kBriefingTooLongToWrap = 7003U,
    kInvalidFraction = 5001U,
    kInvalidAmbientLightValue = 5002U,
    kUnableToFindRoute = 6001U,
    kListError = 666U,
    kNoErrorMessageDeclared = 1000U,
};
} // namespace Gta2Error
