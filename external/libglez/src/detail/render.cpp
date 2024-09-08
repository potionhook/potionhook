/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/detail/render.hpp>
#include <glez/detail/font.hpp>
#include <cstring>
#include <glez/detail/program.hpp>
#include <glez/detail/record.hpp>

static GLuint current_texture{ 0 };

namespace glez::detail::render
{

void begin()
{
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT |
                 GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_FOG_COORD_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_INDEX_ARRAY);

    current_texture = 0;

    program::begin();
}

void end()
{
    program::draw();
    program::reset();
    program::end();
    glPopClientAttrib();
    glPopAttrib();
}

void bind(GLuint texture)
{
    if (current_texture != texture)
    {
        if (!detail::record::isReplaying)
        {
            program::draw();
            program::reset();
        }
        current_texture = texture;
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}
} // namespace glez::detail::render
