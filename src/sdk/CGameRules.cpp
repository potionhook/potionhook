/*
 * CGameRules.cpp
 *
 *  Created on: Aug 06, 2023
 *      Author: Bintr
 */

//-----------------------------------------------------------------------------
// Purpose: Return which Halloween scenario is currently running
//-----------------------------------------------------------------------------
CGameRules::HalloweenScenarioType CGameRules::GetHalloweenScenario() const
{
    if (!const_cast<CGameRules *>(this)->IsHolidayActive(kHoliday_Halloween))
        return HALLOWEEN_SCENARIO_NONE;

    return m_halloweenScenario;
}

static bool BIsCvarIndicatingHolidayIsActive(int iCvarValue, /*EHoliday*/ int eHoliday)
{
    if (iCvarValue == 0)
        return false;

    // Unfortunately Holidays are not a proper bitfield
    switch (eHoliday)
    {
    case CGameRules::kHoliday_TFBirthday:
        return iCvarValue == CGameRules::kHoliday_TFBirthday;
    case CGameRules::kHoliday_Halloween:
        return iCvarValue == CGameRules::kHoliday_Halloween || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoonOrValentines;
    case CGameRules::kHoliday_Christmas:
        return iCvarValue == CGameRules::kHoliday_Christmas;
    case CGameRules::kHoliday_Valentines:
        return iCvarValue == CGameRules::kHoliday_Valentines || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoonOrValentines;
    case CGameRules::kHoliday_MeetThePyro:
        return iCvarValue == CGameRules::kHoliday_MeetThePyro;
    case CGameRules::kHoliday_FullMoon:
        return iCvarValue == CGameRules::kHoliday_FullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoonOrValentines;
    case CGameRules::kHoliday_HalloweenOrFullMoon:
        return iCvarValue == CGameRules::kHoliday_Halloween || iCvarValue == CGameRules::kHoliday_FullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoonOrValentines;
    case CGameRules::kHoliday_HalloweenOrFullMoonOrValentines:
        return iCvarValue == CGameRules::kHoliday_Halloween || iCvarValue == CGameRules::kHoliday_FullMoon || iCvarValue == CGameRules::kHoliday_Valentines || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoon || iCvarValue == CGameRules::kHoliday_HalloweenOrFullMoonOrValentines;
    case CGameRules::kHoliday_AprilFools:
        return iCvarValue == CGameRules::kHoliday_AprilFools;
    case CGameRules::kHoliday_EOTL:
        return iCvarValue == CGameRules::kHoliday_EOTL;
    case CGameRules::kHoliday_CommunityUpdate:
        return iCvarValue == CGameRules::kHoliday_CommunityUpdate;
    default:
        return false;
    }
}

bool TF_IsHolidayActive(/*EHoliday*/ int eHoliday)
{
    if (g_ICvar->FindVar("tf_force_holidays_off")->GetBool())
        return false;

    if (BIsCvarIndicatingHolidayIsActive(g_ICvar->FindVar("tf_forced_holiday")->GetInt(), eHoliday))
        return true;

    if (BIsCvarIndicatingHolidayIsActive(g_ICvar->FindVar("tf_item_based_forced_holiday")->GetInt(), eHoliday))
        return true;

    if ((eHoliday == CGameRules::kHoliday_TFBirthday) && g_ICvar->FindVar("tf_birthday")->GetBool())
        return true;

    if (eHoliday == CGameRules::kHoliday_HalloweenOrFullMoon)
    {
        if (TFGameRules()->IsHolidayMap(CGameRules::kHoliday_Halloween))
            return true;
        if (TFGameRules()->IsHolidayMap(CGameRules::kHoliday_FullMoon))
            return true;
    }

    if (TFGameRules()->IsHolidayMap(eHoliday))
        return true;

    return false;
}