/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"
#if ENABLE_VISUALS
#endif

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(FireGameEvent, void, void *this_, IGameEvent *event)
{
    const char *name = event->GetName();
    original::FireGameEvent(this_, event);
}
} // namespace hooked_methods