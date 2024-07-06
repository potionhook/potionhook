/*
 * CatBot.cpp
 *
 *  Created on: Dec 30, 2017
 *      Author: nullifiedcat
 */

#include <random>
#include <settings/Bool.hpp>
//#include <utility>
#include "CatBot.hpp"
#include "common.hpp"
#include "hack.hpp"
#include "PlayerTools.hpp"
//#include "e8call.hpp"
#include "SettingCommands.hpp"
//#include "glob.h"

namespace hacks::catbot
{

settings::Int requeue_if_ipc_bots_gte{ "cat-bot.requeue-if.ipc-bots-gte", "0" };
static settings::Int requeue_if_humans_lte{ "cat-bot.requeue-if.humans-lte", "0" };
static settings::Int requeue_if_players_lte{ "cat-bot.requeue-if.players-lte", "0" };
static settings::Boolean abandon_instead_requeue{ "cat-bot.abandon-instead", "false" };

static settings::Boolean micspam{ "cat-bot.micspam.enable", "false" };
static settings::Int micspam_on{ "cat-bot.micspam.interval-on", "1" };
static settings::Int micspam_off{ "cat-bot.micspam.interval-off", "0" };

static settings::Boolean auto_crouch{ "cat-bot.auto-crouch", "false" };
static settings::Boolean always_crouch{ "cat-bot.always-crouch", "false" };
static settings::Boolean random_votekicks{ "cat-bot.votekicks", "false" };
static settings::Boolean autovote_map{ "cat-bot.autovote-map", "true" };

settings::Boolean catbotmode{ "cat-bot.enable", "true" };
settings::Boolean anti_motd{ "cat-bot.anti-motd", "true" };

void do_random_votekick()
{
    std::vector<int> targets;
    player_info_s local_info{};

    if (CE_BAD(LOCAL_E) || !GetPlayerInfo(LOCAL_E->m_IDX, &local_info))
    {
        return;
    }

    for (int i = 1; i < g_GlobalVars->maxClients; ++i)
    {
        player_info_s info{};
        if (!GetPlayerInfo(i, &info) || !info.friendsID)
        {
            continue;
        }

        if (g_pPlayerResource->GetTeam(i) != g_pLocalPlayer->team)
        {
            continue;
        }

        if (info.friendsID == local_info.friendsID)
        {
            continue;
        }

        auto &pl = playerlist::AccessData(info.friendsID);
        if (pl.state != playerlist::k_EState::RAGE && pl.state != playerlist::k_EState::DEFAULT && pl.state != playerlist::k_EState::ABANDON)
        {
            continue;
        }

        targets.push_back(info.userID);
    }

    if (targets.empty())
        return;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<unsigned int> dist(0, targets.size());
    int target = targets[dist(mt)];
    player_info_s info{};
    if (!GetPlayerInfo(GetPlayerForUserID(target), &info))
    {
        return;
    }

    hack::ExecuteCommand("callvote kick \"" + std::to_string(target) + " cheating\"");
}

void SendNetMsg(INetMessage &msg)
{

}

class CatBotEventListener2 : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *) override
    {
        // vote for current map if catbot mode and autovote is on
        if (*catbotmode && *autovote_map)
        {
            g_IEngine->ServerCmd("next_map_vote 0");
        }
    }
};

CatBotEventListener2 &listener2()
{
    static CatBotEventListener2 object{};
    return object;
}

Timer timer_votekicks{};
static Timer timer_abandon{};

static int count_ipc = 0;
static std::vector<unsigned int> ipc_list{ 0 };

static bool waiting_for_quit_bool{ false };
static Timer waiting_for_quit_timer{};

static std::vector<unsigned int> ipc_blacklist{};

#if ENABLE_IPC
void update_ipc_data(ipc::user_data_s &data)
{
    data.ingame.bot_count = count_ipc;
}
#endif

Timer level_init_timer{};

Timer micspam_on_timer{};
Timer micspam_off_timer{};

Timer crouchcdr{};
void smart_crouch()
{
    if (g_Settings.bInvalid)
    {
        return;
    }

    if (!current_user_cmd)
    {
        return;
    }

    if (*always_crouch)
    {
        current_user_cmd->buttons |= IN_DUCK;

        if (crouchcdr.test_and_set(10000))
        {
            current_user_cmd->buttons &= ~IN_DUCK;
        }

        return;
    }

    bool foundtar      = false;
    static bool crouch = false;
    if (crouchcdr.test_and_set(2000))
    {
        for (const auto &ent : entity_cache::player_cache)
        {
            if (RAW_ENT(ent)->IsDormant() || ent->m_iTeam() == LOCAL_E->m_iTeam() || !ent->hitboxes.GetHitbox(0) || !player_tools::shouldTarget(ent))
            {
                continue;
            }

            bool failedvis = false;
            for (uint8_t i = 0; i < 18; ++i)
            {
                if (IsVectorVisible(g_pLocalPlayer->v_Eye, ent->hitboxes.GetHitbox(i)->center))
                {
                    failedvis = true;
                }
            }

            if (failedvis)
            {
                continue;
            }

            for (uint8_t i = 0; i < 18; ++i)
            {
                if (!LOCAL_E->hitboxes.GetHitbox(i))
                {
                    continue;
                }

                // Check if they see my hitboxes
                if (!IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(i)->center) && !IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(i)->min) && !IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(i)->max))
                {
                    continue;
                }

                foundtar = true;
                crouch   = true;
            }
        }

        if (!foundtar && crouch)
        {
            crouch = false;
        }
    }

    if (crouch)
    {
        current_user_cmd->buttons |= IN_DUCK;
    }
}

CatCommand print_ammo("debug_print_ammo", "debug",
                      []()
                      {
                          if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
                          {
                              return;
                          }

                          logging::Info("Current slot: %d", re::C_BaseCombatWeapon::GetSlot(RAW_ENT(LOCAL_W)));

                          for (uint8_t i = 0; i < 10; ++i)
                          {
                              logging::Info("Ammo Table %d: %d", i, CE_INT(LOCAL_E, netvar.m_iAmmo + i * 4));
                          }
                      });

static CachedEntity *local_w;
static void cm()
{
    if (!*catbotmode)
    {
        return;
    }

    if (g_Settings.bInvalid)
    {
        return;
    }

    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
    {
        return;
    }

    if (*auto_crouch)
    {
        smart_crouch();
    }

    static const int classes[3]{ tf_spy, tf_sniper, tf_pyro };
}

static Timer unstuck{};
static int unstucks;
void update()
{
    if (g_Settings.bInvalid)
    {
        return;
    }

    if (!*catbotmode)
    {
        return;
    }

    if (CE_BAD(LOCAL_E))
    {
        return;
    }

    if (LOCAL_E->m_bAlivePlayer())
    {
        unstuck.update();
        unstucks = 0;
    }

    if (unstuck.test_and_set(10000))
    {
        unstucks++;
        // Send menuclosed to tell the server that we want to respawn
        hack::command_stack().emplace("menuclosed");
        // If that didn't work, force pick a team and class
        if (unstucks > 3)
        {
            hack::command_stack().emplace("autoteam; join_class sniper");
        }
    }

    if (*micspam)
    {
        if (*micspam_on && micspam_on_timer.test_and_set(*micspam_on * 1000))
        {
            g_IEngine->ClientCmd_Unrestricted("+voicerecord");
        }

        if (*micspam_off && micspam_off_timer.test_and_set(*micspam_off * 1000))
        {
            g_IEngine->ClientCmd_Unrestricted("-voicerecord");
        }
    }

    if (*random_votekicks && timer_votekicks.test_and_set(500))
    {
        do_random_votekick();
    }

    if (timer_abandon.test_and_set(2000) && level_init_timer.check(13000))
    {
        count_ipc = 0;
        ipc_list.clear();
        int count_total = 0;

        for (const auto &ent : entity_cache::valid_ents)
        {
            if (ent->m_Type() != ENTITY_PLAYER || ent == LOCAL_E)
            {
                continue;
            }

            ++count_total;

            player_info_s info{};
            if (!GetPlayerInfo(ent->m_IDX, &info))
            {
                continue;
            }

            if (playerlist::AccessData(info.friendsID).state == playerlist::k_EState::IPC || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::TEXTMODE || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::CAT || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::FRIEND || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::ABANDON)
            {
                ipc_list.push_back(info.friendsID);
                ++count_ipc;
            }
        }

        if (*requeue_if_ipc_bots_gte)
        {
            if (count_ipc >= *requeue_if_ipc_bots_gte)
            {
                // Store local IPC ID and assign to the quit_id variable for later comparisons
                unsigned int local_ipcid = ipc::peer->client_id;
                unsigned int quit_id     = local_ipcid;

                // Iterate all the players marked as bot
                for (auto &id : ipc_list)
                {
                    // We already know we shouldn't quit, so just break out of the loop
                    if (quit_id < local_ipcid)
                    {
                        break;
                    }

                    // Reduce code size
                    auto &peer_mem = ipc::peer->memory;

                    // Iterate all ipc peers
                    for (unsigned char i = 0; i < cat_ipc::max_peers; ++i)
                    {
                        // If that ipc peer is alive and in has the steamid of that player
                        if (!peer_mem->peer_data[i].free && peer_mem->peer_user_data[i].friendid == id)
                        {
                            // Check against blacklist
                            if (std::find(ipc_blacklist.begin(), ipc_blacklist.end(), i) != ipc_blacklist.end())
                            {
                                continue;
                            }

                            // Found someone with a lower ipc id
                            if (i < local_ipcid)
                            {
                                quit_id = i;
                                break;
                            }
                        }
                    }
                }

                // Only quit if you are the player with the lowest ipc id
                if (quit_id == local_ipcid)
                {
                    // Clear blacklist related stuff
                    waiting_for_quit_bool = false;
                    ipc_blacklist.clear();

                    logging::Info("Requeuing/leaving because there are %d local players in game, and requeue_if_ipc_bots_gte is %d.", count_ipc, *requeue_if_ipc_bots_gte);
                    if (*abandon_instead_requeue)
                        tfmm::Abandon();
                    else
                        tfmm::StartQueue();
                    return;
                }
                else
                {
                    if (!waiting_for_quit_bool)
                    {
                        // Waiting for that ipc id to quit, we use this timer in order to blacklist
                        // ipc peers which refuse to quit for some reason
                        waiting_for_quit_bool = true;
                        waiting_for_quit_timer.update();
                    }
                    else
                    {
                        // IPC peer isn't leaving, blacklist for now
                        if (waiting_for_quit_timer.test_and_set(10000))
                        {
                            ipc_blacklist.push_back(quit_id);
                            waiting_for_quit_bool = false;
                        }
                    }
                }
            }
            else
            {
                // Reset Bool because no reason to quit
                waiting_for_quit_bool = false;
                ipc_blacklist.clear();
            }
        }
        if (*requeue_if_humans_lte)
        {
            if (count_total - count_ipc <=  *requeue_if_humans_lte)
            {
                logging::Info("Requeuing/leaving because there are %d non-bots in game, and requeue_if_humans_lte is %d.", count_total - count_ipc, *requeue_if_humans_lte);
                if (*abandon_instead_requeue)
                    tfmm::Abandon();
                else
                    tfmm::StartQueue();
                return;
            }
        }
        if (*requeue_if_players_lte)
        {
            if (count_total <= *requeue_if_players_lte)
            {
                logging::Info("Requeuing/leaving because there are %d total players in game, and requeue_if_players_lte is %d.", count_total, *requeue_if_players_lte);
                if (*abandon_instead_requeue)
                    tfmm::Abandon();
                else
                    tfmm::StartQueue();
                return;
            }
        }
    }
}

void init()
{
    // g_IEventManager2->AddListener(&listener(), "player_death", false);
    g_IEventManager2->AddListener(&listener2(), "vote_maps_changed", false);
}

void level_init()
{
//    deaths = 0;
    level_init_timer.update();
}

void shutdown()
{
    // g_IEventManager2->RemoveListener(&listener());
    g_IEventManager2->RemoveListener(&listener2());
}

static InitRoutine runinit(
    []()
    {
        EC::Register(EC::CreateMove, cm, "cm_catbot", EC::average);
        EC::Register(EC::CreateMove, update, "cm2_catbot", EC::average);
        EC::Register(EC::LevelInit, level_init, "levelinit_catbot", EC::average);
        EC::Register(EC::Shutdown, shutdown, "shutdown_catbot", EC::average);
        init();
    });
} // namespace hacks::catbot
