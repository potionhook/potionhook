/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/detail/font.hpp>
#include <vector>
#include <memory>
#include <cassert>

namespace glez::detail::font
{

std::vector<glez::detail::font::ifont> *cache = nullptr;

void init()
{
    cache = new std::vector<glez::detail::font::ifont>;
}

void shutdown()
{
    delete cache;
}

void ifont::load(const std::string &path, float size)
{
    assert(size > 0);

    atlas = texture_atlas_new(1024, 1024, 1);
    assert(atlas != nullptr);

    m_font = texture_font_new_from_file(atlas, size, path.c_str());
    assert(m_font != nullptr);

    init = true;
}

void ifont::unload()
{
    if (!init)
        return;
    if (atlas)
        texture_atlas_delete(atlas);
    if (m_font)
        texture_font_delete(m_font);
    init = false;
}

void ifont::stringSize(const std::string &string, float *width, float *height)
{
    float penX = 0;

    float size_x = 0;
    float size_y = 0;

    texture_font_load_glyphs(m_font, string.c_str());

    const char *sstring = string.c_str();

    for (size_t i = 0; i < string.size(); ++i)
    {
        // c_str guarantees a NULL terminator
        texture_glyph_t *glyph = texture_font_find_glyph(m_font, &sstring[i]);
        if (glyph == nullptr)
            continue;

        penX += texture_glyph_get_kerning(glyph, &sstring[i]);
        penX += glyph->advance_x;

        if (penX > size_x)
            size_x = penX;

        if (glyph->height > size_y)
            size_y = glyph->height;
    }
    if (width)
        *width = size_x;
    if (height)
        *height = size_y;
}

unsigned create()
{
    for (size_t i = 0; i < cache->size(); ++i)
        if (not(*cache)[i].init)
            return i;
    auto result = cache->size();
    cache->push_back(ifont{});
    return result;
}

ifont &get(unsigned handle)
{
    return cache->at(handle);
}
} // namespace glez::detail::font
