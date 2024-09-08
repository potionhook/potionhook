/*
 * votelogger.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <boost/algorithm/string.hpp>
#include <settings/Bool.hpp>
#include <cstdlib>
#include <ctime>
#include <random>
#include "CatBot.hpp"
#include "votelogger.hpp"

static settings::Boolean vote_kicky{ "votelogger.autovote.yes", "true" };
static settings::Boolean vote_kickn{ "votelogger.autovote.no", "true" };
static settings::Boolean vote_rage_vote{ "votelogger.autovote.no.rage", "false" };
static settings::Boolean chat{ "votelogger.chat", "true" };
static settings::Boolean chat_partysay{ "votelogger.chat.partysay", "false" };
static settings::Boolean chat_casts{ "votelogger.chat.casts", "false" };
static settings::Boolean f2pleaseimnotbot{ "votelogger.f2please", "false" };
static settings::Boolean legitmode{ "votelogger.legitmode", "false" };
static settings::Boolean micspam_after_vote{ "votelogger.micspam-after-kick", "false" };
static settings::Boolean saywhenimkickingaskid{ "votelogger.kicksay", "false" };
static settings::Boolean chat_casts_f1_only{ "votelogger.chat.casts.f1-only", "true" };
static settings::Boolean requeue_on_kick{ "votelogger.requeue-on-kick", "false" };
// Leave party and crash, useful VAC hosting
static settings::Boolean abandon_and_crash_on_kick{ "votelogger.restart-on-kick", "false" };

namespace votelogger
{
static bool was_local_player{ false };
static bool was_local_player_caller{ false };
static Timer local_kick_timer{};

static const std::vector<std::string> f2_phrases =
{
    "f2??",
    "BRO WHY KICK",
    "bro",
    "vote no please",
    "f2 please"
};

static const std::vector<std::string> f1_phrases =
{
    "f1 bot",
    "f1 cheater",
    "hes a bot",
    "vote yes",
    "f1 hacks"
};

static const std::vector<std::string> friendly_f1 =
{
    "kick bot",
    "kick the bot",
    "pls kick them",
    "kick",
    "kick the cheater"
};

static std::string getRandomPhrase(const std::vector<std::string>& phrases)
{
    static std::random_device rd;
    static std::mt19937 gen(rd()); // actual good way to seed.

    std::uniform_int_distribution<> dis(0, phrases.size() - 1);
    return phrases[dis(gen)];
}

static void vote_rage_back()
{
    static Timer attempt_vote_time;
    char cmd[40];
    player_info_s info{};
    std::vector<int> targets;

    if (!g_IEngine->IsInGame() || !attempt_vote_time.test_and_set(1000))
        return;

    for (const auto &ent : entity_cache::player_cache)
    {
        // TO DO: m_bEnemy check only when you can't vote off players from the opposite team
        if (ent->m_bEnemy() || ent == LOCAL_E)
            continue;

        if (!GetPlayerInfo(ent->m_IDX, &info))
            continue;

        auto &pl = playerlist::AccessData(info.friendsID);
        if (pl.state == playerlist::k_EState::RAGE)
            targets.emplace_back(info.userID);
    }
    if (targets.empty())
        return;

    std::snprintf(cmd, sizeof(cmd), "callvote kick \"%d cheating\"", targets[UniformRandomInt(0, targets.size() - 1)]);
    g_IEngine->ClientCmd_Unrestricted(cmd);
}

// Call Vote RNG
struct CVRNG
{
    std::string content;
    unsigned int time{};
    Timer timer{};
};

static CVRNG vote_command;
void dispatchUserMessage(bf_read &buffer, int type)
{
    switch (type)
    {
    case 45:
    {
        // Vote setup Failed
        int reason = buffer.ReadByte();
        int cooldown = buffer.ReadShort();
        int delay = 4;

        if (reason == 2) // VOTE_FAILED_RATE_EXCEEDED
            delay = cooldown;

        hacks::catbot::timer_votekicks -= Timer::sec_to_ms(delay);
        break;
    }
    case 46:
    {
        // TODO: Add always vote no/vote no on friends. Cvar is "vote option2"
        was_local_player_caller = false;
        was_local_player = false;
        int team = buffer.ReadByte();
        int vote_id = buffer.ReadLong();
        int caller = buffer.ReadByte();
        char reason[64];
        char name[64];
        buffer.ReadString(reason, 64, false, nullptr);
        buffer.ReadString(name, 64, false, nullptr);
        auto target = static_cast<unsigned char>(buffer.ReadByte());
        buffer.Seek(0);
        target >>= 1;

        // info is the person getting kicked,
        // info2 is the person calling the kick.
        player_info_s info{}, info2{};
        if (!GetPlayerInfo(target, &info) || !GetPlayerInfo(caller, &info2))
            break;

        using namespace playerlist;

        auto &pl = AccessData(info.friendsID);
        auto &pl_caller = AccessData(info2.friendsID);
        bool friendly_kicked = pl.state != k_EState::RAGE && pl.state != k_EState::DEFAULT && pl.state != k_EState::ABANDON;
        bool friendly_caller = pl_caller.state != k_EState::RAGE && pl_caller.state != k_EState::DEFAULT && pl_caller.state != k_EState::ABANDON;

        auto team_name = teamname(team);
        logging::Info("[%s] Vote called to kick %s [U:1:%u] for %s by %s [U:1:%u]", team_name, info.name, info.friendsID, reason, info2.name, info2.friendsID);
        if (info.friendsID == g_ISteamUser->GetSteamID().GetAccountID())
        {
            if (requeue_on_kick)
            {
                tfmm::StartQueue();
            }

            if (*f2pleaseimnotbot)
            {
                if (*legitmode)
                    chat_stack::Say(getRandomPhrase(f2_phrases).c_str(), true);
                else
                    chat_stack::Say("f2 bro wtf", true);
            }
            was_local_player = true;
            local_kick_timer.update();
        }

        if (info2.friendsID == g_ISteamUser->GetSteamID().GetAccountID())
        {
            if (*saywhenimkickingaskid)
            {
                if (*legitmode)
                    chat_stack::Say(getRandomPhrase(f1_phrases).c_str(), true);
                else
                    chat_stack::Say("f1 cheater/bot", true);
            }

            if (micspam_after_vote)
                g_IEngine->ClientCmd_Unrestricted("+voicerecord");

            was_local_player_caller = true;
        }

        if (friendly_caller && *legitmode && !was_local_player_caller)
        {
            chat_stack::Say(getRandomPhrase(friendly_f1).c_str());
        }

        if (*vote_kickn || *vote_kicky)
        {
            if (*vote_kickn && friendly_kicked)
            {
                if (!legitmode)
                    vote_command = { strfmt("vote %d option2", vote_id).get(), 0 }; // Set the delay to 0
                else
                {
                    vote_command = { strfmt("vote %d option2", vote_id).get(), 1000u + (rand() % 5000) };
                    vote_command.timer.update();
                }

                if (*vote_rage_vote && !friendly_caller)
                    pl_caller.state = k_EState::RAGE;
            }
            else if (*vote_kicky && !friendly_kicked)
            {
                if (!legitmode)
                    vote_command = { strfmt("vote %d option1", vote_id).get(), 0 }; // Set the delay to 0
                else
                {
                    vote_command = { strfmt("vote %d option1", vote_id).get(), 1000u + (rand() % 5000) };
                    vote_command.timer.update();
                }
            }
        }
        if (*chat_partysay)
        {
            char formatted_string[256];
            std::snprintf(formatted_string, sizeof(formatted_string), "[ROSNEHOOK] [%s] votekick called: %s => %s (%s)", team_name, info2.name, info.name, reason);
            re::CTFPartyClient::GTFPartyClient()->SendPartyChat(formatted_string);
        }
#if ENABLE_VISUALS
        if (*chat)
            PrintChat("\x07%06X%s\x01 called a votekick on \x07%06X%s\x01 for the reason \"%s\"", 0xe1ad01, info2.name, 0xe1ad01, info.name, reason);
#endif
        break;
    }
    case 47:
    {
        logging::Info("Vote passed");
        if (was_local_player_caller && micspam_after_vote)
            g_IEngine->ClientCmd_Unrestricted("-voicerecord");
        break;
    }
    case 48:
        logging::Info("Vote failed");
        if (was_local_player && requeue_on_kick)
            tfmm::LeaveQueue();
        if (was_local_player_caller && micspam_after_vote)
            g_IEngine->ClientCmd_Unrestricted("-voicerecord");
        break;
    case 49:
        logging::Info("VoteSetup?");
        [[fallthrough]];
    default:
        break;
    }
}

static bool found_message = false;
void onShutdown(const std::string &message)
{
    if (message.find("Generic_Kicked") == std::string::npos)
    {
        found_message = false;
        return;
    }
    if (local_kick_timer.check(60000) || !was_local_player)
    {
        found_message = false;
        return;
    }
    if (*abandon_and_crash_on_kick)
    {
        found_message = true;
        g_IEngine->ClientCmd_Unrestricted("tf_party_leave");
        local_kick_timer.update();
    }
    else
        found_message = false;
}

static void setup_paint_abandon()
{
    EC::Register(
        EC::Paint,
        []()
        {
            if (!vote_command.content.empty() && vote_command.timer.test_and_set(vote_command.time))
            {
                g_IEngine->ClientCmd_Unrestricted(vote_command.content.c_str());
                vote_command.content = "";
            }
            if (!found_message)
                return;
            if (local_kick_timer.check(60000) || !local_kick_timer.test_and_set(10000) || !was_local_player)
                return;
            if (*abandon_and_crash_on_kick)
                *(int *) nullptr = 0;
        },
        "vote_abandon_restart");
}

static void setup_vote_rage()
{
    EC::Register(EC::CreateMove, vote_rage_back, "vote_rage_back");
}

static void reset_vote_rage()
{
    EC::Unregister(EC::CreateMove, "vote_rage_back");
}

class VoteEventListener : public IGameEventListener
{
public:
    void FireGameEvent(KeyValues *event) override
    {
        if (!*chat_casts || (!*chat_partysay && !*chat))
            return;
        const char *name = event->GetName();
        if (!strcmp(name, "vote_cast"))
        {
            bool vote_option = event->GetInt("vote_option");
            if (*chat_casts_f1_only && vote_option)
                return;
            int eid = event->GetInt("entityid");

            player_info_s info{};
            if (!GetPlayerInfo(eid, &info))
                return;
            if (*chat_partysay)
            {
                char formated_string[256];
                std::snprintf(formated_string, sizeof(formated_string), "[ROSNEHOOK] %s voted %s", info.name, vote_option ? "No" : "Yes");

                re::CTFPartyClient::GTFPartyClient()->SendPartyChat(formated_string);
            }
#if ENABLE_VISUALS
            if (*chat)
            {
                switch (vote_option)
                {
                case true:
                {
                    PrintChat("\x07%06X%s\x01 voted \x07%06XNo\x01", 0xe1ad01, info.name, 0x00ff00);
                    break;
                }
                case false:
                {
                    PrintChat("\x07%06X%s\x01 voted \x07%06XYes\x01", 0xe1ad01, info.name, 0xff0000);
                    break;
                }
                }
            }
#endif
        }
    }
};

static VoteEventListener listener{};
static InitRoutine init(
    []()
    {
        if (*vote_rage_vote)
            setup_vote_rage();
        setup_paint_abandon();

        vote_rage_vote.installChangeCallback(
            [](settings::VariableBase<bool> &var, bool new_val)
            {
                if (new_val)
                    setup_vote_rage();
                else
                    reset_vote_rage();
            });
        g_IGameEventManager->AddListener(&listener, false);
        EC::Register(
            EC::Shutdown, []() { g_IGameEventManager->RemoveListener(&listener); }, "event_shutdown_vote");
    });
} // namespace votelogger
