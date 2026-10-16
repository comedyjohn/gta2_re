"""Which subsystem each file of port/game/ belongs to (file stem, no extension).

Used by metrics.py and by the refactoring plan (port/refactor/), which works through the code one
subsystem at a time and turns each subsystem into a library in phase 6. When a file is split or
renamed, update this map in the same commit.
"""

SUBSYSTEMS = {
    # Code that exists only because the game was decompiled (hooking into the original exe, decomp.me,
    # VC6 workarounds). Deleted or replaced in phase 1.
    "scaffolding": {
        "HookLoader", "decompme_context", "GlobalsRegistry", "Globals", "test", "StdAfx", "crt_stubs",
        "resource",
    },
    # Types, fixed point, angles, random numbers, pools, bit sets: no game knowledge.
    "core": {
        "types", "Function", "fix16", "Fix16_Point", "Fix16_Rect", "ang16", "angle", "rng", "BitSet32",
        "rgb", "Pool", "Ptr32", "memory", "enums",
    },
    # File formats and loading: style (.sty), text (.gxt), car info (.gci), chunks, player data, the
    # registry and debug settings.
    "data": {
        "file", "chunk", "gtx_0x106C", "text_0x14", "CarInfo_808", "registry", "debug", "PlyDat_2BC0",
    },
    # The map (.gmp), routes, path finding, lighting, tile animations, traffic lights.
    "world": {
        "map_0x370", "RouteFinder", "PathFinder_2FD4", "TileAnim_2", "TrafficLights_194", "Light_1D4CC",
        "Light_28",
    },
    # Sprites (the physical body of every moving thing), the sprite grid and collision.
    "sprites": {
        "sprite", "SpriteGrid_400", "collide", "sprite_flags", "collision_event_type", "collision_target_type",
    },
    # Objects, weapons, explosions, particles, doors, cranes, crushers, generators.
    "objects": {
        "Object_2C_Pool", "Object_3C", "Object_3C_Pool", "Object_5C", "Object_8", "Object_8_Pool",
        "ObjectDefinitions_8CA8", "Explosion_30", "explosion_type", "Particle_4C", "Particle_8",
        "particle_flags", "Door_38", "Door_4D4", "Cranes", "Crushers", "Generators", "Weapon_30", "Weapon_8",
    },
    # Cars, their physics and AI, chases, trains and buses, garages, taxis.
    "vehicles": {
        "Car_BC", "Car_10", "CarPhysics_B0", "CarAI_78", "CarChaseTaskTable_500", "PublicTransport",
        "Garage_48", "Taxi_4", "car_ai_flags", "car_chase_state", "car_despawn_status", "car_flags",
        "car_model_flag", "car_score_tier", "car_wreck",
    },
    # Pedestrians: the ped, its body (Char_B4), groups, lists, patrol routes.
    "peds": {
        "Ped", "char", "Char_Pool", "PedGroup", "Ped_List_4", "PedRefTable_7F8", "PatrolRoutePool_1D7E",
        "char_b4_flags", "ped_attack_target_type", "ped_class", "ped_death_cause", "ped_fire_mode",
        "ped_flags", "ped_graphic_type", "ped_jump_over_mode", "ped_lifetime", "ped_move_target_type",
    },
    # Police, ambulances, fire engines, gangs, crime reports.
    "services": {
        "Police_38", "Police_7B8", "EmergencyCrewPool_1E0", "Ambulance_110", "Firefighters", "Gang",
        "CrimeReportQueue_CC", "emergency_car_kill_flag",
    },
    # The mission script VM (.scr) and its threads.
    "script": {
        "miss2_0x11C", "Miss2_25C", "miss2_8", "ScriptManager_C1EA8", "ScriptStackFramePool_25C", "miss2_xyz",
    },
    # The game loop, players, cameras, scores, bonuses, game modes, input and replays.
    "game": {
        "Game_0x40", "Player", "GameSession_578", "PlayerCrimeStats_3C", "PlayerScoreTracker_36C",
        "BonusTracker_1C0", "TagGame_28", "Camera", "InputRecorder_67F8B0", "input", "keybrd_0x204",
        "Mike_A80", "accuracy_event", "bonus_event_type", "multiplayer_game_type",
    },
    # Everything that draws: renderer front end, textures, HUD, front-end menus, fonts, credits, movies,
    # the debug overlay.
    "presentation": {
        "Hud", "Frontend", "MapRenderer", "SpriteRenderer_1C", "TextureCache_15D8", "Draw", "dma_video",
        "gbh_graphics", "KanjiFont_8EC", "CreditsLine_6C", "CreditsText_FD22", "RollingDigitCounter_38",
        "FpsCounter_54", "ProfilerTimer_C", "Bink", "ImGuiDebug", "ExplodingScore_100", "hud_message_priority",
        "credits_line_category",
    },
    # Sound logic and the sample manager over Miles.
    "audio": {
        "sound_obj", "cSampleManager", "root_sound", "SoundObject_10", "CollisionSoundQueue_C88", "voice_line",
    },
    # Network games (DirectPlay) and the lobby dialog.
    "network": {"NetPlay", "Network_20324", "network_data"},
    # Start-up, the window procedure, the main loop, fatal errors.
    "app": {"winmain", "error"},
}
