/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <string>
#include <limits>

namespace glez
{

class texture
{
public:
    inline explicit texture(std::string path) : path(std::move(path))
    {
    }
    ~texture();

    inline int getWidth() const
    {
        return width;
    }

    inline int getHeight() const
    {
        return height;
    }

    inline unsigned getHandle() const
    {
        return handle;
    }

    inline bool isLoaded() const
    {
        return loaded;
    }

    inline bool canLoad() const
    {
        return canload;
    }

    void load();
    void unload();

    const std::string path;

protected:
    int width{ 0 };
    int height{ 0 };
    bool loaded{ false };
    bool canload{ true };

    unsigned handle{ std::numeric_limits<unsigned>::max() };
};
} // namespace glez