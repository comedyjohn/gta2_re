#pragma once

#include "Function.hpp"

class Ped;
class Fix16;

class PedGroup
{
  public:
    // 9.6f 0x4038E0
    void set_in_use_4038E0()
    {
        field_40_in_use = 1;
    }

    // 9.6f 0x4038F0
    char_type get_in_use_4038F0()
    {
        return field_40_in_use;
    }

    void sub_4C8E60();
    static void sub_4C8E80();
    void ClearGroupData_4C8E90();
    void Reset_4C8EF0();
    void ResetGroupObjectives_4C8F20();
    void add_ped_to_end_of_list_4C8F90(Ped* pPed);
    void replace_leader_4C8FE0(Ped* pPed);
    bool PurgeMembersInCars_4C9040();
    char_type AreAllMembersOffScreen_4C9150();
    void ResetMembersToFollowLeader_4C91B0();
    bool IsLeaderInCar_4C9210();
    bool IsLeaderEnteringCarOrUnknown5_4C9220();
    void KillEntireGroup_4C9240();
    void DisbandGroup_4C92A0();
    void DestroyGroup_4C93A0();
    void DisbandGroupDueToAttack_4C94E0(Ped* pAttacker);
    void PromoteMemberToLeader_4C9680(u8 idx);
    void RemovePed_4C9970(Ped* pPed);
    void add_ped_leader_4C9B10(Ped* ptr);
    void add_ped_to_list_4C9B30(Ped* ptr, u8 idx);
    void MergeWithOtherGroup_4C9B60(Ped* pPed);
    Ped* sub_4C9ED0();
    void CoordinateGroupCarEntry_4C9F00();
    u32 sub_4CA3E0();
    Ped* FindFarthestMember_4CA3F0(Fix16* pFoundDistance);
    void UpdateFormation_4CA4B0();
    void UpdateMemberAIState_4CA5E0(u8 idx);
    void UpdateMemberTightFollowState_4CA820(u8 idx);
    bool IsAllMembersInSomeCar_4CAA20();
    char_type HasNoActiveMembers_4CAAE0();
    char_type AreAllMembersOnFoot_4CAB80();
    bool IsMemberTooFarFromLeader_4CAC20(u8 idx);
    bool IsLeaderCloseToTargetCar_4CAD40();
    Ped* FindNearestOtherMember_4CAE80(u8 idx);
    static void ResetAllGroups_4CB080();
    static PedGroup* New_4CB0D0();
    PedGroup();
    ~PedGroup() = default; // 0x4cb870

    // 9.6f 0x433360
    void SetCounts_433360(u8 count)
    {
        field_36_count = count;
        field_34_count = count;
    }

    s32 Get_F3C_433370()
    {
        return field_3C;
    }

    char_type field_0;
    bool
        field_1_wait_for_members; // while set, UpdateFormation makes the leader wait (objective 9) for a member who has fallen too far behind
    char_type field_2_unused; // padding
    char_type field_3_unused; // padding
    Ped* field_4_ped_list[9];
    s32 field_28;
    Ped* field_2C_ped_leader;
    s32 field_30_panicking; // UpdateFormation recomputes it as "the leader or a member is panicking"; merging groups, a leader in state 3/5/10 and the paramedics' revive step also set it. A dword in the original, so not a bool
    u8 field_34_count;
    char_type field_35_unused;
    char_type field_36_count;
    char_type field_37_unused;
    s32 field_38_group_type;
    s32 field_3C;
    char_type field_40_in_use;
    char_type field_41_unused;
    char_type field_42_unused;
    char_type field_43_unused;
};