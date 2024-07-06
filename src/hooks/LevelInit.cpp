/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <settings/Bool.hpp>
#if ENABLE_VISUALS
#include <menu/GuiInterface.hpp>
#endif
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#include "AntiAntiAim.hpp"
#include <random>



namespace hooked_methods
{
DEFINE_HOOKED_METHOD(LevelInit, void, void *this_, const char *name)
{
    firstcm = true;
    // nav::init = false;
    playerlist::Save();
#if ENABLE_VISUALS
#if ENABLE_GUI
    gui::onLevelLoad();
#endif
    ConVar *holiday = g_ICvar->FindVar("tf_forced_holiday");
#endif
    hacks::anti_anti_aim::resolver_map.clear();
    g_IEngine->ClientCmd_Unrestricted("exec cat_matchexec");
    entity_cache::array.reserve(2048);
    chat_stack::Reset();
    original::LevelInit(this_, name);
    EC::run(EC::LevelInit);
#if ENABLE_IPC
    if (ipc::peer)
        ipc::peer->memory->peer_user_data[ipc::peer->client_id].ts_connected = time(nullptr);
#endif
}
} // namespace hooked_methods