/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/detail/font.hpp>
#include <glez/detail/program.hpp>
#include <glez/detail/render.hpp>
#include <glez/detail/texture.hpp>
#include <glez/record.hpp>
#include <glez/detail/record.hpp>

namespace glez
{

void init(int width, int height)
{
    detail::program::init(width, height);
}

void shutdown()
{
    detail::font::shutdown();
    detail::program::shutdown();
    detail::texture::shutdown();
}

void resize(int width, int height)
{
    detail::program::resize(width, height);
}

void begin()
{
    detail::render::begin();
}

void end()
{
    detail::render::end();
}

void preInit()
{
    detail::font::init();
    detail::texture::init();
}
} // namespace glez