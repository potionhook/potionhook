/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/draw.hpp>
#include <glez/font.hpp>
#include <glez/detail/render.hpp>
#include <glez/detail/program.hpp>
#include <vertex-buffer.h>
#include <glez/detail/font.hpp>
#include <cstring>
#include <glez/detail/texture.hpp>
#include <cmath>
#include <glez/detail/record.hpp>

namespace indices
{

static GLuint rectangle[6] = { 0, 1, 2, 2, 3, 0 };
static GLuint triangle[3]  = { 0, 1, 2 };
} // namespace indices

void internal_draw_string(float x, float y, const std::string &string,
                          texture_font_t *fnt, glez::rgba color, float *width,
                          float *height)
{
    float pen_x  = x;
    float pen_y  = y + fnt->height / 1.5f;
    float size_y = 0;

    if (glez::detail::record::currentRecord)
    {
        glez::detail::record::currentRecord->bindFont(fnt);
    }
    else
    {
        if (fnt->atlas->id == 0)
        {
            glGenTextures(1, &fnt->atlas->id);
        }

        glez::detail::render::bind(fnt->atlas->id);

        if (fnt->atlas->dirty)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, fnt->atlas->width,
                         fnt->atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE,
                         fnt->atlas->data);
            fnt->atlas->dirty = 0;
        }
    }

    const char *sstring = string.c_str();

    bool skipped{ false };

    for (size_t i = 0; i < string.size(); ++i)
    {
        texture_glyph_t *glyph = texture_font_find_glyph(fnt, &sstring[i]);
        if (glyph == NULL)
        {
            texture_font_load_glyph(fnt, &sstring[i]);
            if (!skipped)
                --i;
            skipped = true;
            continue;
        }
        skipped = false;
        glez::detail::render::vertex vertices[4];
        for (auto &vertex : vertices)
        {
            vertex.color = color;
            vertex.mode =
                static_cast<int>(glez::detail::program::mode::FREETYPE);
        }

        if (i > 0)
        {
            x += texture_glyph_get_kerning(glyph, &sstring[i - 1]);
        }

        float x0 = (int) (pen_x + glyph->offset_x);
        float y0 = (int) (pen_y - glyph->offset_y);
        float x1 = (int) (x0 + glyph->width);
        float y1 = (int) (y0 + glyph->height);
        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;

        vertices[0].position = { x0, y0 };
        vertices[0].uv       = { s0, t0 };

        vertices[1].position = { x0, y1 };
        vertices[1].uv       = { s0, t1 };

        vertices[2].position = { x1, y1 };
        vertices[2].uv       = { s1, t1 };

        vertices[3].position = { x1, y0 };
        vertices[3].uv       = { s1, t0 };

        pen_x += glyph->advance_x;
        // pen_x = (int) pen_x + 1;
        if (glyph->height > size_y)
            size_y = glyph->height;

        if (glez::detail::record::currentRecord)
            glez::detail::record::currentRecord->store(vertices, 4,
                                                       indices::rectangle, 6);
        else
            vertex_buffer_push_back(glez::detail::program::buffer, vertices, 4,
                                    indices::rectangle, 6);
    }

    if (width)
        *width = int(pen_x - x);
    if (height)
        *height = int(size_y);
}

namespace glez::draw
{

void line(float x, float y, float dx, float dy, rgba color, float thickness)
{
    // Dirty
    x += 0.5f;
    y += 0.5f;

    float length = sqrtf(dx * dx + dy * dy);
    dx *= (length - 1.0f) / length;
    dy *= (length - 1.0f) / length;

    detail::render::vertex vertices[4];

    for (auto &vertex : vertices)
    {
        vertex.mode  = static_cast<int>(detail::program::mode::PLAIN);
        vertex.color = color;
    }

    float nx = dx;
    float ny = dy;

    float ex = x + dx;
    float ey = y + dy;

    if (length <= 1.0f)
        return;

    nx /= length;
    ny /= length;

    float th = thickness;

    nx *= th * 0.5f;
    ny *= th * 0.5f;

    float px = ny;
    float py = -nx;

    vertices[2].position = { float(x) - nx + px, float(y) - ny + py };
    vertices[1].position = { float(x) - nx - px, float(y) - ny - py };
    vertices[3].position = { ex + nx + px, ey + ny + py };
    vertices[0].position = { ex + nx - px, ey + ny - py };

    if (detail::record::currentRecord)
        detail::record::currentRecord->store(vertices, 4, indices::rectangle,
                                             6);
    else
        ftgl::vertex_buffer_push_back(detail::program::buffer, vertices, 4,
                                      indices::rectangle, 6);
}

void rect(float x, float y, float w, float h, rgba color)
{
    detail::render::vertex vertices[4];

    for (auto &vertex : vertices)
    {
        vertex.mode  = static_cast<int>(detail::program::mode::PLAIN);
        vertex.color = color;
    }

    vertices[0].position = { x, y };
    vertices[1].position = { x, y + h };
    vertices[2].position = { x + w, y + h };
    vertices[3].position = { x + w, y };

    if (detail::record::currentRecord)
        detail::record::currentRecord->store(vertices, 4, indices::rectangle,
                                             6);
    else
        ftgl::vertex_buffer_push_back(detail::program::buffer, vertices, 4,
                                      indices::rectangle, 6);
}

void triangle(float x, float y, float x2, float y2, float x3, float y3, rgba color)
{
    detail::render::vertex vertices[3];

    for (auto &vertex : vertices)
    {
        vertex.mode  = static_cast<int>(detail::program::mode::PLAIN);
        vertex.color = color;
    }

    vertices[0].position = { x, y };
    vertices[1].position = { x2, y2 };
    vertices[2].position = { x3, y3 };

    if (detail::record::currentRecord)
        detail::record::currentRecord->store(vertices, 3, indices::rectangle,
                                             3);
    else
        ftgl::vertex_buffer_push_back(detail::program::buffer, vertices, 3,
                                      indices::rectangle, 3);
}

void rect_outline(float x, float y, float w, float h, rgba color,
                  float thickness)
{
    rect(x, y, w, 1, color);
    rect(x, y, 1, h, color);
    rect(x + w - 1, y, 1, h, color);
    rect(x, y + h - 1, w, 1, color);
}

void circle(float x, float y, float radius, rgba color, float thickness,
            int steps)
{
    float px = 0;
    float py = 0;
    for (int i = 0; i <= steps; ++i)
    {
        float ang = 2 * float(M_PI) * (float(i) / steps);
        if (!i)
            ang = 2 * float(M_PI);
        if (i)
            line(px, py, x - px + radius * cos(ang), y - py + radius * sin(ang),
                 color, thickness);
        px = x + radius * cos(ang);
        py = y + radius * sin(ang);
    }
}

void string(float x, float y, const std::string &string, font &font, rgba color,
            float *width, float *height)
{
    if (!font.isLoaded())
        font.load();

    auto fnt               = glez::detail::font::get(font.getHandle()).m_font;
    fnt->rendermode        = RENDER_NORMAL;
    fnt->outline_thickness = 0.0f;
    internal_draw_string(x, y, string, fnt, color, width, height);
}

void outlined_string(float x, float y, const std::string &string, font &font,
                     rgba color, rgba outline, float *width, float *height)
{
    if (!font.isLoaded())
        font.load();

    auto fnt               = glez::detail::font::get(font.getHandle()).m_font;
    fnt->rendermode        = RENDER_OUTLINE_POSITIVE;
    fnt->outline_thickness = 1.0f;
    internal_draw_string(x, y, string, fnt, outline, width, height);
    fnt->rendermode        = RENDER_NORMAL;
    fnt->outline_thickness = 0.0f;
    internal_draw_string(x, y, string, fnt, color, width, height);
}

void rect_textured(float x, float y, float w, float h, rgba color,
                   texture &texture, float tx, float ty, float tw, float th,
                   float angle)
{
    if (!texture.isLoaded())
        texture.load();

    if (!texture.canLoad())
        return;

    auto &tex = detail::texture::get(texture.getHandle());
    if (glez::detail::record::currentRecord)
        glez::detail::record::currentRecord->bindTexture(&tex);
    else
        tex.bind();

    detail::render::vertex vertices[4];

    for (auto &vertex : vertices)
    {
        vertex.mode  = static_cast<int>(detail::program::mode::TEXTURED);
        vertex.color = color;
    }

    vertices[0].position = { x, y };
    vertices[1].position = { x, y + h };
    vertices[2].position = { x + w, y + h };
    vertices[3].position = { x + w, y };

    if (angle != 0.0f)
    {
        float cx = x + float(w) / 2.0f;
        float cy = y + float(h) / 2.0f;

        for (auto &v : vertices)
        {
            float ox = v.position.x;
            float oy = v.position.y;

            v.position.x =
                cx + cosf(angle) * (ox - cx) - sinf(angle) * (oy - cy);
            v.position.y =
                cy + sinf(angle) * (ox - cx) + cosf(angle) * (oy - cy);
        }
    }

    float s0 = float(tx) / texture.getWidth();
    float s1 = float(tx + tw) / texture.getWidth();
    float t0 = float(ty) / texture.getHeight();
    float t1 = float(ty + th) / texture.getHeight();

    vertices[0].uv = { s0, t0 };
    vertices[1].uv = { s0, t1 };
    vertices[2].uv = { s1, t1 };
    vertices[3].uv = { s1, t0 };

    if (detail::record::currentRecord)
        detail::record::currentRecord->store(vertices, 4, indices::rectangle,
                                             6);
    else
        ftgl::vertex_buffer_push_back(detail::program::buffer, vertices, 4,
                                      indices::rectangle, 6);
}
} // namespace glez::draw
