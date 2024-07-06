/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <settings/Float.hpp>
#include "HookedMethods.hpp"

static settings::Float override_fov{ "visual.fov", "0" };
static settings::Button zoom_key{ "visual.zoom-key", "<null>" };
static settings::Int zoom_fov{ "visual.zoom-key.fov", "20" };
bool zoomed_last_tick{ false };
bool user_sensitivity_ratio_set{ true };
float zoom_sensitivity_ratio_user;

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(OverrideView, void, void *this_, CViewSetup *setup)
{
    original::OverrideView(this_, setup);

    if (!isHackActive() || g_Settings.bInvalid || CE_BAD(LOCAL_E))
        return;

    bool zoomed = setup->fov < CE_INT(LOCAL_E, netvar.iDefaultFOV) || g_pLocalPlayer->bZoomed;
    if (*no_zoom && zoomed)
    {
        auto fov   = g_ICvar->FindVar("fov_desired");
        setup->fov = override_fov ? *override_fov : fov->GetInt();
    }
    else if (override_fov && !zoomed)
    {
        setup->fov = *override_fov;
    }
    if (zoom_key && zoom_key.isKeyDown())
    {
        auto default_fov               = g_ICvar->FindVar("default_fov");
        float sens_factor              = (*zoom_fov / setup->fov) * (setup->fov / default_fov->GetFloat());
        setup->fov                     = *zoom_fov;
        g_CHUD->GetSensitivityFactor() = sens_factor;
        zoomed_last_tick               = true;
    }
    else if (zoomed_last_tick)
    {
        g_CHUD->GetSensitivityFactor() = 0.0f;
        zoomed_last_tick               = false;
    }

    if (spectator_target)
    {
        CachedEntity *spec = ENTITY(spectator_target);
        if (CE_GOOD(spec) && !spec->m_bAlivePlayer())
        {
            setup->origin = spec->m_vecOrigin() + CE_VECTOR(spec, netvar.vViewOffset);
            // why not spectate yourself
            if (spec == LOCAL_E)
                setup->angles = CE_VAR(spec, netvar.m_angEyeAnglesLocal, QAngle);
            else
                setup->angles = CE_VAR(spec, netvar.m_angEyeAngles, QAngle);
        }
        if (g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_SPACE))
            spectator_target = 0;
    }

    draw::fov = setup->fov;

    auto zoom_sensitivity_ratio = g_ICvar->FindVar("zoom_sensitivity_ratio");

    if (g_pLocalPlayer->bZoomed)
    {
        static bool last_zoom_state = false;
        bool current_zoom_state     = no_zoom && no_scope;
        if (current_zoom_state != last_zoom_state)
        {
            if (user_sensitivity_ratio_set)
                zoom_sensitivity_ratio_user = zoom_sensitivity_ratio->GetFloat();

            // Both requirements are true, so change the zoom_sensitivity_ratio to 4
            if (no_zoom && no_scope)
            {
                zoom_sensitivity_ratio->SetValue(4);
                user_sensitivity_ratio_set = false;
            }
            // No removing zoom, so user zoom_sensitivity_ratio will be reset to what they had it to on zoom
            else
            {
                if (!user_sensitivity_ratio_set)
                {
                    zoom_sensitivity_ratio->SetValue(zoom_sensitivity_ratio_user);
                    user_sensitivity_ratio_set = true;
                }
            }
        }
        last_zoom_state = current_zoom_state;
    }
}
static InitRoutine override_init(
    []()
    {
        EC::Register(
            EC::Shutdown, []() { g_CHUD->GetSensitivityFactor() = 0.0f; }, "override_shutdown");
    });
} // namespace hooked_methods