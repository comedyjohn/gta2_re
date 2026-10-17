#include "debug.hpp"
#include "core/function.hpp"
#include "registry.hpp"
#include <cstdio>

DEFINE_GLOBAL(bool, gDoInfiniteLives, 0x67D4C9);
DEFINE_GLOBAL(bool, gDebugLimitRecycling, 0x67D4CA);
DEFINE_GLOBAL(bool, gDebugDoInvulnerable, 0x67D4CB);
DEFINE_GLOBAL(bool, gDebugLogInput, 0x67D4CC);
DEFINE_GLOBAL(bool, gDebugSkipRight, 0x67D4E4);
DEFINE_GLOBAL(bool, gDebugSkipMission, 0x67D4E5);
DEFINE_GLOBAL(bool, gDebugLogCollisions, 0x67D4E6);
DEFINE_GLOBAL(bool, gDebugSkipBottom, 0x67D4E7);
DEFINE_GLOBAL(bool, gDebugSkipQuitConfirm, 0x67D4E8);
DEFINE_GLOBAL(bool, gDebugDoIainTest, 0x67D4E9);
DEFINE_GLOBAL(bool, gDebugSkipDraw, 0x67D4EA);
DEFINE_GLOBAL(bool, gDebugDoReleaseReplay, 0x67D4EB);
DEFINE_GLOBAL(bool, gDebugSkipTrafficLights, 0x67D4EC);
DEFINE_GLOBAL(bool, gDebugDoCornerWindow, 0x67D4EE);
DEFINE_GLOBAL(bool, gDebugSkipDummies, 0x67D4EF);
DEFINE_GLOBAL(bool, gDebugDoLoadSavegame, 0x67D4F0);
DEFINE_GLOBAL(bool, gDebugDoShowHorn, 0x67D4F2);
DEFINE_GLOBAL(bool, gDebugIgnoreReplayHeader, 0x67D4F3);
DEFINE_GLOBAL(bool, gDebugPlayReplay, 0x67D4F4);
DEFINE_GLOBAL(bool, gDebugDontGetCarBack, 0x67D4F5);
DEFINE_GLOBAL(bool, gDebugDoTest, 0x67D4F8);
DEFINE_GLOBAL(bool, gDebugSkipPolice, 0x67D4F9);
DEFINE_GLOBAL(bool, gDebugDoShowTrafficLightsInfo, 0x67D4FA);
DEFINE_GLOBAL(bool, gDebugExplodingScoresOff, 0x67D4FB);
DEFINE_GLOBAL(bool, gDebugShowBriefNumber, 0x67D504);
DEFINE_GLOBAL(bool, gDebugSkipSlopes, 0x67D505);
DEFINE_GLOBAL(bool, gDebugSkipUser, 0x67D506);
DEFINE_GLOBAL(bool, gDebugSkipFireEngines, 0x67D53A);
DEFINE_GLOBAL(bool, gDebugSkipFrontend, 0x67D53B);
DEFINE_GLOBAL(bool, gDebugIsFrench, 0x67D53C);
DEFINE_GLOBAL(bool, gDebugDoBrianTest, 0x67D544);
DEFINE_GLOBAL(bool, gDebugSkipLid, 0x67D546);
DEFINE_GLOBAL(bool, gDebugShowPlayerNames, 0x67D54C);
DEFINE_GLOBAL(bool, gDebugKeepWeaponsAfterDeath, 0x67D54D);
DEFINE_GLOBAL(bool, gDebugSkipWindowCheck, 0x67D54E);
DEFINE_GLOBAL(bool, gDebugDoShowPhysics, 0x67D54F);
DEFINE_GLOBAL(bool, gDebugSkipTrains, 0x67D550);
DEFINE_GLOBAL(bool, gDebugSkipBuses, 0x67D558);
DEFINE_GLOBAL(bool, gDebugDoShowIds, 0x67D559);
DEFINE_GLOBAL(bool, gDebugDoPolice1, 0x67D568);
DEFINE_GLOBAL(bool, gDebugDoPolice2, 0x67D569);
DEFINE_GLOBAL(bool, gDebugDoPolice3, 0x67D56A);
DEFINE_GLOBAL(bool, gDebugSkipTop, 0x67D574);
DEFINE_GLOBAL(bool, gDebugSkipRecycling, 0x67D575);
DEFINE_GLOBAL(bool, gDoShowInput, 0x67D576);
DEFINE_GLOBAL(bool, gDebugSkipSkidmarks, 0x67D585);
DEFINE_GLOBAL(bool, gDebugNoAnnoyingChars, 0x67D586);
DEFINE_GLOBAL(bool, gDebugDoShowImaginary, 0x67D588);
DEFINE_GLOBAL(bool, gDebugDoShowCamera, 0x67D58A);
DEFINE_GLOBAL(bool, gDebugSkipReplaySyncCheck, 0x67D594);
DEFINE_GLOBAL(bool, gDebugDoShowJuncIds, 0x67D5B0);
DEFINE_GLOBAL(bool, gDebugLogRandomExtra, 0x67D5BC);
DEFINE_GLOBAL(bool, gDebugConstantReplaySave, 0x67D5C4);
DEFINE_GLOBAL(bool, gDebugDoBlood, 0x67D5C5);
DEFINE_GLOBAL(bool, gDebugDoMike, 0x67D5CC);
DEFINE_GLOBAL(bool, gDebugShowHiddenFaces, 0x67D5CD);
DEFINE_GLOBAL(bool, gDebugLogRandom, 0x67D5FC);
DEFINE_GLOBAL(bool, gDebugDoShowInstruments, 0x67D64C);
DEFINE_GLOBAL(bool, gDebugSkipParticles, 0x67D64D);
DEFINE_GLOBAL(bool, gDebugSkipTiles, 0x67D655);
DEFINE_GLOBAL(bool, gDebugGetAllWeapons, 0x67D684);
DEFINE_GLOBAL(bool, gDoMissLogging, 0x67D6BC);
DEFINE_GLOBAL(bool, gShowCycle, 0x67D6BD);
DEFINE_GLOBAL(bool, gDebugSkipAudio, 0x67D6BE);
DEFINE_GLOBAL(bool, gDebugSkipLeft, 0x67D6BF);
DEFINE_GLOBAL(bool, gDebugLogDirectinput, 0x67D6C0);
DEFINE_GLOBAL(bool, gDebugDoSyncCheck, 0x67D6C1);
DEFINE_GLOBAL(bool, gDebugDo3dSound, 0x67D6C2);
DEFINE_GLOBAL(bool, gDebugSkipAmbulance, 0x67D6C9);
DEFINE_GLOBAL(bool, gDebugDoShowObjectIds, 0x67D6CA);
DEFINE_GLOBAL(bool, gDebugDoFreeShopping, 0x67D6CD);
DEFINE_GLOBAL(bool, gDebugDoDebugKeys, 0x67D6CF);
DEFINE_GLOBAL(bool, gDoTextIdTest, 0x67D6D0);
DEFINE_GLOBAL(bool, gDebugLogRoutefinder, 0x67D6D1);
DEFINE_GLOBAL(bool, gDebugDoShowTiming, 0x67D6DC);
DEFINE_GLOBAL(bool, gDebugDoExitAfterReplay, 0x67D6E4);
DEFINE_GLOBAL(bool, gDebugDoShowCollisionBox, 0x67D6E5);
DEFINE_GLOBAL(bool, gDoShowCounter, 0x67D6E6);
DEFINE_GLOBAL(bool, gDebugShowAllArrows, 0x67D6E7);
DEFINE_GLOBAL(bool, gDebugDoKillPhonesOnAnswer, 0x67D6E8);
DEFINE_GLOBAL(bool, gUnusedDebugFlag, 0x67D654);
DEFINE_GLOBAL(bool, gCheatJailCardAndAmmo, 0x67D56B);

// 0x4ABBD0
void Debug::Init()
{
    gDebugPlayReplay = gRegistry.GetDebugSetting("play_replay");
    gDebugConstantReplaySave = gRegistry.GetDebugSetting("constant_replay_save");
    gDebugSkipDummies = gRegistry.GetDebugSetting("skip_dummies");
    gDebugDoTest = gRegistry.GetDebugSetting("do_test");
    gDebugSkipMission = gRegistry.GetDebugSetting("skip_mission");
    gShowCycle = gRegistry.GetDebugSetting("show_cycle");
    gDebugDoBrianTest = gRegistry.GetDebugSetting("do_brian_test");
    gDebugDoIainTest = gRegistry.GetDebugSetting("do_iain_test");
    gDebugSkipTiles = gRegistry.GetDebugSetting("skip_tiles");
    gDoShowCounter = gRegistry.GetDebugSetting("do_show_counters");
    gDebugDoShowCamera = gRegistry.GetDebugSetting("do_show_camera");
    gDoShowInput = gRegistry.GetDebugSetting("do_show_input");
    gDebugDoShowTiming = gRegistry.GetDebugSetting("do_show_timing");
    gDebugDoShowCollisionBox = gRegistry.GetDebugSetting("do_show_collision_box");
    gDebugDoShowPhysics = gRegistry.GetDebugSetting("do_show_physics");
    gDebugDoShowImaginary = gRegistry.GetDebugSetting("do_show_imaginary");
    gDebugSkipUser = gRegistry.GetDebugSetting("skip_user");
    gDebugSkipTrafficLights = gRegistry.GetDebugSetting("skip_traffic_lights");
    gDebugSkipRecycling = gRegistry.GetDebugSetting("skip_recycling");
    gDebugLogCollisions = gRegistry.GetDebugSetting("log_collisions");
    gDebugDoShowTrafficLightsInfo = gRegistry.GetDebugSetting("do_show_traffic_lights_info");
    gDebugDoShowIds = gRegistry.GetDebugSetting("do_show_ids");
    gDebugLimitRecycling = gRegistry.GetDebugSetting("limit_recycling");
    gDebugNoAnnoyingChars = gRegistry.GetDebugSetting("no_annoying_chars");
    gDebugSkipSlopes = gRegistry.GetDebugSetting("skip_slopes");
    gDebugSkipLeft = gRegistry.GetDebugSetting("skip_left");
    gDebugSkipRight = gRegistry.GetDebugSetting("skip_right");
    gDebugSkipTop = gRegistry.GetDebugSetting("skip_top");
    gDebugSkipBottom = gRegistry.GetDebugSetting("skip_bottom");
    gDebugSkipLid = gRegistry.GetDebugSetting("skip_lid");
    gDebugLogRoutefinder = gRegistry.GetDebugSetting("log_routefinder");
    gDebugDoMike = gRegistry.GetDebugSetting("do_mike");
    gDebugSkipParticles = gRegistry.GetDebugSetting("skip_particles");
    gDebugShowHiddenFaces = gRegistry.GetDebugSetting("show_hidden_faces");
    gDebugGetAllWeapons = gRegistry.GetDebugSetting("get_all_weapons");
    gDebugDoExitAfterReplay = gRegistry.GetDebugSetting("do_exit_after_replay");
    gDebugDontGetCarBack = gRegistry.GetDebugSetting("dont_get_car_back");
    gDebugDoShowInstruments = gRegistry.GetDebugSetting("do_show_instruments");
    gDebugSkipAmbulance = gRegistry.GetDebugSetting("skip_ambulance");
    gDebugSkipPolice = gRegistry.GetDebugSetting("skip_police");
    gDebugSkipFrontend = gRegistry.GetDebugSetting("skip_frontend");
    gDebugDoInvulnerable = gRegistry.GetDebugSetting("do_invulnerable");
    gDebugShowAllArrows = gRegistry.GetDebugSetting("show_all_arrows");
    gDebugDoShowHorn = gRegistry.GetDebugSetting("do_show_horn");
    gDebugKeepWeaponsAfterDeath = gRegistry.GetDebugSetting("keep_weapons_after_death");
    gDebugSkipSkidmarks = gRegistry.GetDebugSetting("skip_skidmarks");
    gDebugDoShowJuncIds = gRegistry.GetDebugSetting("do_show_junc_ids");
    gDebugDoCornerWindow = gRegistry.GetDebugSetting("do_corner_window");
    gDoInfiniteLives = gRegistry.GetDebugSetting("do_infinite_lives");
    gDebugDoLoadSavegame = gRegistry.GetDebugSetting("do_load_savegame");
    gDebugSkipAudio = gRegistry.GetDebugSetting("skip_audio");
    gDebugDoDebugKeys = gRegistry.GetDebugSetting("do_debug_keys");
    gDebugLogRandom = gRegistry.GetDebugSetting("log_random");
    gDebugLogRandomExtra = gRegistry.GetDebugSetting("log_random_extra");
    gDebugLogInput = gRegistry.GetDebugSetting("log_input");
    gDebugLogDirectinput = gRegistry.GetDebugSetting("log_directinput");
    gDebugIgnoreReplayHeader = gRegistry.GetDebugSetting("ignore_replay_header");
    gDebugSkipTrains = gRegistry.GetDebugSetting("skip_trains");
    gDebugSkipBuses = gRegistry.GetDebugSetting("skip_buses");
    gDebugSkipQuitConfirm = gRegistry.GetDebugSetting("skip_quit_confirm");
    gDebugDoSyncCheck = gRegistry.GetDebugSetting("do_sync_check");
    gDebugSkipFireEngines = gRegistry.GetDebugSetting("skip_fire_engines");
    gDebugShowBriefNumber = gRegistry.GetDebugSetting("show_brief_number");
    gDebugSkipWindowCheck = gRegistry.GetDebugSetting("skip_window_check");
    gDebugSkipReplaySyncCheck = gRegistry.GetDebugSetting("skip_replay_sync_check");
    gDebugDoShowObjectIds = gRegistry.GetDebugSetting("do_show_object_ids");
    gDebugDoKillPhonesOnAnswer = gRegistry.GetDebugSetting("do_kill_phones_on_answer");
    gDoMissLogging = gRegistry.GetDebugSetting("do_miss_logging");
    gDoTextIdTest = gRegistry.GetDebugSetting("do_text_id_test");
    gDebugDoPolice1 = gRegistry.GetDebugSetting("do_police_1");
    gDebugDoPolice2 = gRegistry.GetDebugSetting("do_police_2");
    gDebugDoPolice3 = gRegistry.GetDebugSetting("do_police_3");
    gDebugSkipDraw = gRegistry.GetDebugSetting("skip_draw");
    gDebugDoFreeShopping = gRegistry.GetDebugSetting("do_free_shopping");
    gDebugDoReleaseReplay = gRegistry.GetDebugSetting("do_release_replay");
    gDebugDoBlood = gRegistry.GetDebugSetting("do_blood");
    gDebugDo3dSound = gRegistry.GetSoundSettting("do_3d_sound");
    gDebugIsFrench = HaveOnlyFrenchGtx();
    gDebugShowPlayerNames = gRegistry.SetNetworkSetting("show_player_names", 1) != 0;
}

// 0x4AC200
bool Debug::HaveOnlyFrenchGtx()
{
    bool bEGtxExists = false;
    bool bFgxtExists = false;
    bool bGGtxExists = false;
    bool bIGtxExists = false;
    bool bJGtxExists = false;
    bool bSGtxExists = false;

    FILE* fEnglish = fopen("data\\e.gxt", "rb");
    if (fEnglish)
    {
        fclose(fEnglish);
        bEGtxExists = true;
    }

    FILE* fFrench = fopen("data\\f.gxt", "rb");
    if (fFrench)
    {
        fclose(fFrench);
        bFgxtExists = true;
    }

    FILE* fGerman = fopen("data\\g.gxt", "rb");
    if (fGerman)
    {
        fclose(fGerman);
        bGGtxExists = true;
    }

    FILE* fItalian = fopen("data\\i.gxt", "rb");
    if (fItalian)
    {
        fclose(fItalian);
        bIGtxExists = true;
    }

    FILE* fJapanese = fopen("data\\j.gxt", "rb");
    if (fJapanese)
    {
        fclose(fJapanese);
        bJGtxExists = true;
    }

    FILE* fSpanish = fopen("data\\s.gxt", "rb");
    if (fSpanish)
    {
        fclose(fSpanish);
        bSGtxExists = true;
    }

    bool r = (!bEGtxExists && bFgxtExists == true && !bGGtxExists && !bIGtxExists && !bJGtxExists && !bSGtxExists);

    return r;
}

DEFINE_GLOBAL(Debug, gDebug, 0x67D52C);
