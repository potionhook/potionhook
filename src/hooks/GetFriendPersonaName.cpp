/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <settings/String.hpp>
#include "HookedMethods.hpp"
#include "PlayerTools.hpp"


int getRng(int min, int max)
{
    static std::random_device rd;
    std::uniform_int_distribution<int> unif(min, max);
    static std::mt19937 rand_engine(rd());

    int x = unif(rand_engine);
    return x;
}

namespace hooked_methods
{
std::string GetFriendPersonaName_name;
DEFINE_HOOKED_METHOD(GetFriendPersonaName, const char *, ISteamFriends *this_, CSteamID steam_id)
{
    if (!isHackActive())
        GetFriendPersonaName_name = "";
    return (!GetFriendPersonaName_name.empty() ? GetFriendPersonaName_name.c_str() : original::GetFriendPersonaName(this_, steam_id));
}

} // namespace hooked_methods