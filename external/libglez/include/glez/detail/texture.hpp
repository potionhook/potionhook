/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <freetype-gl.h>
#include <string>
#include <limits>

namespace glez::detail::texture
{

struct texture
{
    void bind();
    bool load(const std::string &path);
    void unload();

    bool init{ false };
    bool bound{ false };

    int width{ 0 };
    int height{ 0 };

    GLuint id;
    GLubyte *data;
};

void init();
void shutdown();

unsigned create();
texture &get(unsigned handle);
} // namespace glez::detail::texture