/*

Warp.hpp

*/

#pragma once
#include "settings/Bool.hpp"
class INetMessage;

namespace hacks::warp
{
extern bool in_rapidfire;
extern bool in_warp;
extern settings::Boolean dodge_projectile;
void SendNetMessage(INetMessage &msg);
void CL_SendMove_hook();
} // namespace hacks::warp
