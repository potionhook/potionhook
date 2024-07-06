/*
 * Aimbot.hpp
 *
 *  Created on: Oct 8, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

class ConVar;
class IClientEntity;

namespace hacks::aimbot
{
extern unsigned last_target_ignore_timer;

// Functions used to calculate aimbot data, and if already calculated use it
Vector PredictEntity(CachedEntity *entity);

// Functions called by other functions for when certain game calls are run
void Reset();

// Stuff to make storing functions easy
bool IsAiming();
CachedEntity *CurrentTarget();
bool ShouldAim();
CachedEntity *RetrieveBestTarget(bool aimkey_state);
bool IsTargetStateGood(CachedEntity *entity);
bool Aim(CachedEntity *entity);
void DoAutoshoot(CachedEntity *target = nullptr);
int NotVisibleHitbox(CachedEntity *target, int preferred);
std::vector<Vector> GetHitpointsVischeck(CachedEntity *ent, int hitbox);
bool small_box_checker(CachedEntity* target_entity);
float ProjectileHitboxSize(int projectile_size);
int AutoHitbox(CachedEntity *target);
bool HitscanSpecialCases(CachedEntity *target_entity, int weapon_case);
bool ProjectileSpecialCases(CachedEntity *target_entity, int weapon_case);
int BestHitbox(CachedEntity *target);
bool IsHitboxMedium(int hitbox);
int ClosestHitbox(CachedEntity *target);
void DoSlowAim(Vector &inputAngle);
bool UpdateAimkey();
} // namespace hacks::aimbot
