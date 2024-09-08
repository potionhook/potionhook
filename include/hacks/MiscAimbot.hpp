/*

MiscAimbot.hpp

*/

#pragma once
#include "common.hpp"
namespace hacks::misc_aimbot
{
std::pair<CachedEntity *, Vector> FindBestEnt(bool teammate, bool Predict, bool zcheck, bool fov_check, float range = 1500.0f);
void DoSlowAim(Vector &input_angle, float speed = 5.0f);
bool ShouldHitBuilding(CachedEntity *ent);
bool ShouldChargeAim();
} // namespace hacks::misc_aimbot
