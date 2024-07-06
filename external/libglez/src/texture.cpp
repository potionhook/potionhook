/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/texture.hpp>
#include <glez/detail/texture.hpp>

namespace glez
{

texture::~texture()
{
    if (loaded)
        unload();
}

void texture::load()
{
    if (loaded)
        return;
    handle        = detail::texture::create();
    auto &texture = detail::texture::get(handle);
    if (canload = texture.load(path))
    {
        width  = texture.width;
        height = texture.height;
    }
    loaded = true;
}

void texture::unload()
{
    auto &texture = detail::texture::get(handle);
    texture.unload();
    loaded = false;
}
} // namespace glez