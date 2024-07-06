/*
 * CreateMove.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"
#include "MiscTemporary.hpp"
#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include <hacks/AntiAntiAim.hpp>
#include "HookTools.hpp"
#include "Misc.hpp"
#include "HookedMethods.hpp"
#include "nospread.hpp"
#include "Warp.hpp"

static settings::Boolean roll_speedhack{ "misc.roll-speedhack", "false" };
static settings::Boolean forward_speedhack{ "misc.roll-speedhack.forward", "false" };
settings::Boolean engine_pred{ "misc.engine-prediction", "true" };

class CMoveData;
namespace engine_prediction
{
static Vector original_origin;

void RunEnginePrediction(IClientEntity *ent, CUserCmd *ucmd)
{
    if (!ent)
        return;

    typedef void (*SetupMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, class IMoveHelper *, CMoveData *);
    typedef void (*FinishMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, CMoveData *);

    void **predictionVtable = *(void ***) g_IPrediction;

    auto oSetupMove  = (SetupMoveFn) (*(unsigned *) (predictionVtable + 19));
    auto oFinishMove = (FinishMoveFn) (*(unsigned *) (predictionVtable + 20));
    // CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr + 0x1F69C0C);  CMoveData movedata {};
    auto object     = std::make_unique<char[]>(165);
    auto *pMoveData = (CMoveData *) object.get();

    // Backup
    float frameTime = g_GlobalVars->frametime;
    float curTime   = g_GlobalVars->curtime;
    int tickcount   = g_GlobalVars->tickcount;
    original_origin = ent->GetAbsOrigin();

    CUserCmd defaultCmd{};
    if (ucmd == nullptr)
        ucmd = &defaultCmd;

    // Set Usercmd for prediction
    NET_VAR(ent, CURR_CUSERCMD_PTR, CUserCmd *) = ucmd;

    // Set correct CURTIME
    g_GlobalVars->curtime   = g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
    g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
    *g_PredictionRandomSeed = MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;

    // Run The Prediction
    g_IGameMovement->StartTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));
    oSetupMove(g_IPrediction, ent, ucmd, nullptr, pMoveData);
    g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer *>(ent), pMoveData);
    oFinishMove(g_IPrediction, ent, ucmd, pMoveData);
    g_IGameMovement->FinishTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));

    // Reset User CMD
    NET_VAR(ent, CURR_CUSERCMD_PTR, CUserCmd *) = nullptr;

    g_GlobalVars->frametime = frameTime;
    g_GlobalVars->curtime   = curTime;
    g_GlobalVars->tickcount = tickcount;

    // Adjust tickbase
    NET_INT(ent, netvar.nTickBase)++;
}
// Restore Origin
void FinishEnginePrediction(IClientEntity *ent, CUserCmd *ucmd)
{
    const_cast<Vector &>(ent->GetAbsOrigin()) = original_origin;
    original_origin.Invalidate();
}
} // namespace engine_prediction

void PrecalculateCanShoot()
{
    auto weapon = LOCAL_W;
    // Check if player and weapon are good
    if (CE_BAD(LOCAL_E) || CE_BAD(weapon))
    {
        calculated_can_shoot = false;
        return;
    }

    // flNextPrimaryAttack without reload
    static float next_attack = 0.0f;
    // Last shot fired using weapon
    static float last_attack = 0.0f;
    // Last weapon used
    static CachedEntity *last_weapon = nullptr;
    float server_time                = (float) (CE_INT(LOCAL_E, netvar.nTickBase)) * g_GlobalVars->interval_per_tick;
    float new_next_attack            = CE_FLOAT(weapon, netvar.flNextPrimaryAttack);
    float new_last_attack            = CE_FLOAT(weapon, netvar.flLastFireTime);

    // Reset everything if using a new weapon/shot fired
    if (new_last_attack != last_attack || last_weapon != weapon)
    {
        next_attack = new_next_attack;
        last_attack = new_last_attack;
        last_weapon = weapon;
    }
    // Check if we can shoot
    calculated_can_shoot = next_attack <= server_time;
}

namespace hooked_methods
{
void speedHack(CUserCmd *cmd)
{
    float speed;
    if (cmd->buttons & IN_DUCK && (g_pLocalPlayer->flags & FL_ONGROUND) && !(cmd->buttons & IN_ATTACK) && !HasCondition<TFCond_Charging>(LOCAL_E))
    {
        speed                     = Vector{ cmd->forwardmove, cmd->sidemove, 0.0f }.Length();
        static float prevspeedang = 0.0f;
        if (fabs(speed) > 0.0f)
        {

            if (forward_speedhack)
            {
                cmd->forwardmove *= -1.0f;
                cmd->sidemove *= -1.0f;
                cmd->viewangles.x = 91;
            }
            Vector vecMove(cmd->forwardmove, cmd->sidemove, 0.0f);

            vecMove *= -1;
            float flLength = vecMove.Length();
            Vector angMoveReverse{};
            VectorAngles(vecMove, angMoveReverse);
            cmd->forwardmove = -flLength;
            cmd->sidemove    = 0.0f; // Move only backwards, no sidemove
            float res        = g_pLocalPlayer->v_OrigViewangles.y - angMoveReverse.y;
            while (res > 180)
                res -= 360;
            while (res < -180)
                res += 360;
            if (res - prevspeedang > 90.0f)
                res = (res + prevspeedang) / 2;
            prevspeedang                     = res;
            cmd->viewangles.y                = res;
            cmd->viewangles.z                = 90.0f;
            g_pLocalPlayer->bUseSilentAngles = true;
        }
    }
}

DEFINE_HOOKED_METHOD(CreateMove, bool, void *this_, float input_sample_time, CUserCmd *cmd)
{
    g_Settings.is_create_move = true;
    bool time_replaced, ret;
    float curtime_old, servertime;

    current_user_cmd = cmd;
    EC::run(EC::CreateMoveEarly);
    ret = original::CreateMove(this_, input_sample_time, cmd);

    if (!cmd)
    {
        g_Settings.is_create_move = false;
        return ret;
    }

    // Disabled because this causes EXTREME aimbot inaccuracy
    // Actually don't disable it. It causes even more inaccuracy
    if (!cmd->command_number)
    {
        g_Settings.is_create_move = false;
        return ret;
    }

    tickcount++;

    if (!isHackActive())
    {
        g_Settings.is_create_move = false;
        return ret;
    }

    if (!g_IEngine->IsInGame() || g_IEngine->IsLevelMainMenuBackground())
    {
        g_Settings.bInvalid       = true;
        g_Settings.is_create_move = false;
        return true;
    }

    if (current_user_cmd && current_user_cmd->command_number)
        last_cmd_number = current_user_cmd->command_number;

    time_replaced = false;
    curtime_old   = g_GlobalVars->curtime;

    if (CE_GOOD(LOCAL_E))
    {
        servertime            = (float) CE_INT(LOCAL_E, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
        g_GlobalVars->curtime = servertime;
        time_replaced         = true;
    }
    if (g_Settings.bInvalid)
        entity_cache::Invalidate();

    //	PROF_BEGIN();
    // Do not update if in warp, since the entities will stay identical either way
    if (!hacks::warp::in_warp)
    {
        entity_cache::Update();
    }
    //	PROF_END("Entity Cache updating");
    {
        g_pPlayerResource->Update();
    }
    {
        g_pLocalPlayer->Update();
    }
    PrecalculateCanShoot();
    if (firstcm)
    {
        DelayTimer.update();
        EC::run(EC::FirstCM);
        firstcm = false;
    }
    g_Settings.bInvalid = false;

    if (CE_GOOD(LOCAL_E))
    {
        if (!g_pLocalPlayer->life_state && CE_GOOD(LOCAL_W))
        {
            // Walkbot can leave game.
            if (!g_IEngine->IsInGame())
            {
                g_Settings.is_create_move = false;
                return ret;
            }
            g_pLocalPlayer->isFakeAngleCM = false;
            static int fakelag_queue      = 0;

            if (CE_GOOD(LOCAL_E))
                if (!hacks::nospread::is_syncing && (fakelag_amount || (hacks::antiaim::force_fakelag && hacks::antiaim::isEnabled())))
                {
                    // Do not fakelag when trying to attack
                    bool do_fakelag = true;
                    switch (GetWeaponMode())
                    {
                    case weapon_melee:
                    {
                        if (g_pLocalPlayer->weapon_melee_damage_tick)
                            do_fakelag = false;
                        break;
                    }
                    case weapon_hitscan:
                    {
                        if ((CanShoot() || isRapidFire(RAW_ENT(LOCAL_W))) && current_user_cmd->buttons & IN_ATTACK)
                            do_fakelag = false;
                        break;
                    }
                    default:
                        break;
                    }

                    if (fakelag_midair && g_pLocalPlayer->flags & FL_ONGROUND)
                        do_fakelag = false;

                    if (do_fakelag)
                    {
                        int fakelag_amnt = *fakelag_amount > 1 ? *fakelag_amount : 1;
                        *bSendPackets    = fakelag_amnt == fakelag_queue;
                        if (*bSendPackets)
                            g_pLocalPlayer->isFakeAngleCM = true;
                        fakelag_queue++;
                        if (fakelag_queue > fakelag_amnt)
                            fakelag_queue = 0;
                    }
                }
            {
                hacks::antiaim::ProcessUserCmd(cmd);
            }
        }
    }
    else
        return false;

    {
        EC::run(EC::CreateMove_NoEnginePred);

        if (engine_pred)
        {
            engine_prediction::RunEnginePrediction(RAW_ENT(LOCAL_E), current_user_cmd);
            g_pLocalPlayer->UpdateEye();
        }

        if (hacks::warp::in_warp)
            EC::run(EC::CreateMoveWarp);
        else
            EC::run(EC::CreateMove);
    }
    if (time_replaced)
        g_GlobalVars->curtime = curtime_old;
    g_Settings.bInvalid = false;
    {
        chat_stack::OnCreateMove();
    }

    // TODO Auto Steam Friend

#if ENABLE_IPC
    {
        static Timer ipc_update_timer{};
        // playerlist::DoNotKillMe();
        if (ipc_update_timer.test_and_set(10000))
            ipc::UpdatePlayerlist();
    }
#endif
    if (CE_GOOD(LOCAL_E))
    {
        if (roll_speedhack)
            speedHack(cmd);
        else
        {
            if (g_pLocalPlayer->bUseSilentAngles)
            {

                float speed, yaw;
                Vector ang, vsilent;
                vsilent.x = cmd->forwardmove;
                vsilent.y = cmd->sidemove;
                vsilent.z = cmd->upmove;
                speed     = std::hypot(vsilent.x, vsilent.y);
                VectorAngles(vsilent, ang);
                yaw                 = DEG2RAD(ang.y - g_pLocalPlayer->v_OrigViewangles.y + cmd->viewangles.y);
                cmd->forwardmove    = cos(yaw) * speed;
                cmd->sidemove       = sin(yaw) * speed;
                float clamped_pitch = fabsf(fmodf(cmd->viewangles.x, 360.0f));
                if (clamped_pitch >= 90 && clamped_pitch <= 270)
                    cmd->forwardmove = -cmd->forwardmove;

                ret = false;
            }
        }
        g_pLocalPlayer->UpdateEnd();
    }

    g_Settings.is_create_move = false;
    if (nolerp)
    {
        static const ConVar *pUpdateRate = g_pCVar->FindVar("cl_updaterate");
        if (!pUpdateRate)
            pUpdateRate = g_pCVar->FindVar("cl_updaterate");
        else
        {
            auto ratio      = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
            auto lerptime   = std::max(cl_interp->GetFloat(), ratio / (sv_maxupdaterate ? sv_maxupdaterate->GetFloat() : cl_updaterate->GetFloat()));
            cmd->tick_count = TIME_TO_TICKS(CE_FLOAT(LOCAL_E, netvar.m_flSimulationTime) + lerptime);
        }
    }
    return ret;
}

void WriteCmd(IInput *input, CUserCmd *cmd, int sequence_nr)
{
    // Write the usercmd
    GetVerifiedCmds(input)[sequence_nr % VERIFIED_CMD_SIZE].m_cmd = *cmd;
    GetVerifiedCmds(input)[sequence_nr % VERIFIED_CMD_SIZE].m_crc = GetChecksum(cmd);
    GetCmds(input)[sequence_nr % VERIFIED_CMD_SIZE]               = *cmd;
}

// This gets called before the other CreateMove, but since we run original first in here all the stuff gets called after normal CreateMove is done
DEFINE_HOOKED_METHOD(CreateMoveInput, void, IInput *this_, int sequence_nr, float input_sample_time, bool arg3)
{
    bSendPackets = reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(__builtin_frame_address(1)) - 8);
    // Call original function, includes Normal CreateMove
    original::CreateMoveInput(this_, sequence_nr, input_sample_time, arg3);

    CUserCmd *cmd = nullptr;
    if (this_ && GetCmds(this_) && sequence_nr > 0)
        cmd = this_->GetUserCmd(sequence_nr);

    if (!cmd)
        return;

    current_late_user_cmd = cmd;

    if (!isHackActive())
    {
        WriteCmd(this_, current_late_user_cmd, sequence_nr);
        return;
    }

    if (!g_IEngine->IsInGame())
    {
        WriteCmd(this_, current_late_user_cmd, sequence_nr);
        return;
    }


    // Run EC
    EC::run(EC::CreateMoveLate);

    // Restore prediction
    if (CE_GOOD(LOCAL_E) && engine_prediction::original_origin.IsValid())
        engine_prediction::FinishEnginePrediction(RAW_ENT(LOCAL_E), current_late_user_cmd);

    // Write the usercmd
    WriteCmd(this_, current_late_user_cmd, sequence_nr);
}
} // namespace hooked_methods
