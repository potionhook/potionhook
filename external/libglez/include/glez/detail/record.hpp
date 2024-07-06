/*
  Created on 19.06.18.
*/

#pragma once

#include <vector>
#include <cstdint>
#include <glez/texture.hpp>
#include <glez/font.hpp>
#include "render.hpp"
#include "../../../ftgl/vertex-buffer.h"
#include "texture.hpp"
#include "font.hpp"
#include "../../../ftgl/freetype-gl.h"

namespace glez::detail::record
{

class RecordedCommands
{
public:
    struct segment
    {
        std::size_t start{ 0 };
        std::size_t size{ 0 };
        glez::detail::texture::texture *texture{ nullptr };
        ftgl::texture_font_t *font{ nullptr };
    };

    RecordedCommands();
    ~RecordedCommands();

    void reset();
    void store(glez::detail::render::vertex *vertices, size_t vcount,
               uint32_t *indices, size_t icount);
    void bindTexture(glez::detail::texture::texture *tx);
    void bindFont(ftgl::texture_font_t *font);
    void render();
    void end();

protected:
    void cutSegment();

    ftgl::vertex_buffer_t *vertex_buffer{};
    std::vector<segment> segments{};
    segment current{};
};

extern RecordedCommands *currentRecord;
extern bool isReplaying;

} // namespace glez::detail::record