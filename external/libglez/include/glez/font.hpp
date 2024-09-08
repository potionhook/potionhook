/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <string>
#include <limits>

namespace glez
{

class font
{
public:
    inline font(std::string path, float size)
        : path(std::move(path)), size(size), loaded(false)
    {
    }
    ~font();

    inline unsigned getHandle() const
    {
        return handle;
    }

    inline bool isLoaded() const
    {
        return loaded;
    }

    void load();
    void unload();
    void stringSize(const std::string &string, float *width, float *height);

    const std::string path;
    const float size;

protected:
    bool loaded{ false };

    unsigned handle{ std::numeric_limits<unsigned>::max() };
};
} // namespace glez
