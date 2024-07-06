/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

namespace glez
{

struct rgba
{
    rgba() = default;
    inline constexpr rgba(int r, int g, int b)
        : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(1.0f)
    {
    }
    inline constexpr rgba(int r, int g, int b, int a)
        : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f)
    {
    }

    float r;
    float g;
    float b;
    float a;
};

namespace color
{

constexpr rgba white(255, 255, 255);
constexpr rgba black(0, 0, 0);

constexpr rgba red(255, 0, 0);
constexpr rgba green(0, 255, 0);
constexpr rgba blue(0, 0, 255);
} // namespace color
} // namespace glez