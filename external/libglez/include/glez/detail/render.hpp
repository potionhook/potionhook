/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <freetype-gl.h>
#include <glez/color.hpp>

namespace glez::detail::render
{

struct vertex
{
    ftgl::vec2 position;
    ftgl::vec2 uv;
    rgba color;
    int mode;
};

void begin();
void end();

void bind(GLuint texture);
} // namespace glez::detail::render