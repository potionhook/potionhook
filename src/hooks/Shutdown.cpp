/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#include "votelogger.hpp"


namespace hooked_methods
{
DEFINE_HOOKED_METHOD(Shutdown, void, INetChannel *this_, const char *reason)
{
    g_Settings.bInvalid = true;
    logging::Info("Disconnect: %s", reason);
    g_IEngine->ClientCmd_Unrestricted("alias sniperragefromlegitbot");
#if ENABLE_IPC
    ipc::UpdateServerAddress(true);
#endif
    original::Shutdown(this_, reason);
    ignoredc = false;
    hacks::autojoin::OnShutdown();
    std::string message = reason;
    votelogger::onShutdown(message);
}

} // namespace hooked_methods
