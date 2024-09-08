/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <freetype-gl.h>
#include <limits>
#include <string>

namespace glez::detail::font
{

struct ifont
{
    void load(const std::string &path, float size);
    void unload();
    void stringSize(const std::string &string, float *width, float *height);

    bool init{ false };

    texture_font_t *m_font{ nullptr };
    texture_atlas_t *atlas{ nullptr };
};

void init();
void shutdown();

unsigned create();
ifont &get(unsigned handle);
} // namespace glez::detail::font
