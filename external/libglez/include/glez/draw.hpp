/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <string>
#include "color.hpp"
#include "font.hpp"
#include "texture.hpp"

namespace glez::draw
{

void line(float x, float y, float dx, float dy, rgba color, float thickness);
void rect(float x, float y, float w, float h, rgba color);
void triangle(float x, float y, float x2, float y2, float x3, float y3, rgba color);
void rect_outline(float x, float y, float w, float h, rgba color,
                  float thickness);
void rect_textured(float x, float y, float w, float h, rgba color,
                   texture &texture, float tx, float ty, float tw, float th,
                   float angle);
void circle(float x, float y, float radius, rgba color, float thickness,
            int steps);

void string(float x, float y, const std::string &string, font &font, rgba color,
            float *width, float *height);
void outlined_string(float x, float y, const std::string &string, font &font,
                     rgba color, rgba outline, float *width, float *height);
} // namespace glez::draw
