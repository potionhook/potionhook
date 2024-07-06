/*
  Created on 19.06.18.
*/

#include <cstddef>
#include <glez/detail/record.hpp>
#include <glez/record.hpp>
#include <cstring>

namespace glez::detail::record
{

void RecordedCommands::render()
{
    isReplaying = true;
    vertex_buffer_render_setup(vertex_buffer, GL_TRIANGLES);
    for (const auto &i : segments)
    {
        if (i.texture)
        {
            i.texture->bind();
        }
        else if (i.font)
        {
            if (i.font->atlas->id == 0)
            {
                glGenTextures(1, &i.font->atlas->id);
            }

            glez::detail::render::bind(i.font->atlas->id);

            if (i.font->atlas->dirty)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, i.font->atlas->width,
                             i.font->atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE,
                             i.font->atlas->data);
                i.font->atlas->dirty = 0;
            }
        }
        glDrawElements(GL_TRIANGLES, i.size, GL_UNSIGNED_INT,
                       (void *) (i.start * 4));
    }
    vertex_buffer_render_finish(vertex_buffer);
    isReplaying = false;
}

void RecordedCommands::store(glez::detail::render::vertex *vertices,
                             size_t vcount, uint32_t *indices, size_t icount)
{
    vertex_buffer_push_back(vertex_buffer, vertices, vcount, indices, icount);
}

RecordedCommands::RecordedCommands()
{
    vertex_buffer =
        vertex_buffer_new("vertex:2f,tex_coord:2f,color:4f,drawmode:1i");
}

RecordedCommands::~RecordedCommands()
{
    vertex_buffer_delete(vertex_buffer);
}

void RecordedCommands::reset()
{
    vertex_buffer_clear(vertex_buffer);
    segments.clear();
    memset(&current, 0, sizeof(current));
}

void RecordedCommands::bindTexture(glez::detail::texture::texture *tx)
{
    if (current.texture != tx)
    {
        cutSegment();
        current.texture = tx;
    }
}

void RecordedCommands::bindFont(ftgl::texture_font_t *font)
{
    if (current.font != font)
    {
        cutSegment();
        current.font = font;
    }
}

void RecordedCommands::cutSegment()
{
    current.size = vertex_buffer->indices->size - current.start;
    if (current.size)
        segments.push_back(current);
    memset(&current, 0, sizeof(segment));
    current.start = vertex_buffer->indices->size;
}

void RecordedCommands::end()
{
    cutSegment();
}

RecordedCommands *currentRecord{ nullptr };
bool isReplaying{ false };

} // namespace glez::detail::record

glez::record::Record::Record()
{
    commands = new glez::detail::record::RecordedCommands{};
}

glez::record::Record::~Record()
{
    delete commands;
}

void glez::record::Record::begin()
{
    detail::record::currentRecord = commands;
    commands->reset();
}

void glez::record::Record::end()
{
    commands->end();
    detail::record::currentRecord = nullptr;
}

void glez::record::Record::replay()
{
    commands->render();
}
