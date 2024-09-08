/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <glez/detail/texture.hpp>
#include <glez/detail/render.hpp>
#include <cassert>
#include <vector>
#include <glez/picopng/picopng.hpp>
#include <memory>

#include <fstream> // required to load the file

static std::vector<glez::detail::texture::texture> *cache{ nullptr };

namespace glez::detail::texture
{

void init()
{
    cache = new std::vector<glez::detail::texture::texture>();
}

void shutdown()
{
    delete cache;
}

void texture::bind()
{
    if (!bound)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        bound = true;
    }

    render::bind(id);
}

bool texture::load(const std::string &path)
{
    std::ifstream file(path.c_str(),
                       std::ios::in | std::ios::binary | std::ios::ate);

    std::streamsize size = 0;
    if (file.seekg(0, std::ios::end).good())
        size = file.tellg();
    if (file.seekg(0, std::ios::beg).good())
        size -= file.tellg();

    if (size < 1)
        return false;

    unsigned char *buffer = new unsigned char[(size_t) size + 1];
    file.read((char *) buffer, size);
    file.close();
    int error = decodePNG(data, width, height, buffer, size);

    // if there's an error, display it and return false to indicate failure
    if (error != 0)
    {
        printf("Error loading texture, error code %i\n", error);
        return false;
    }
    init  = true;
    bound = false;
    id    = 0;
    return true;
}

void texture::unload()
{
    if (bound)
        glDeleteTextures(1, &id);
    if (init)
        delete[] data;
    init = false;
}

texture &get(unsigned handle)
{
    return (*cache)[handle];
}

unsigned create()
{
    for (auto i = 0u; i < cache->size(); ++i)
        if (not(*cache)[i].init)
            return i;
    auto result = cache->size();
    cache->push_back(texture{});
    return result;
}
} // namespace glez::detail::texture
