/*
 * sconvars.hpp
 *
 *  Created on: May 1, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "convar.h"

/*
 * HECK off F1ssi0N
 * I won't make NETWORK HOOKS to deal with this SHIT
 */

namespace sconvar
{
class SpoofedConVar
{
public:
    explicit SpoofedConVar(ConVar *var);
    ConVar *original;
    ConVar *spoof;
};
} // namespace sconvar
