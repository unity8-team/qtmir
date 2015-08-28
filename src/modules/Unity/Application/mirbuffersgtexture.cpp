/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mirbuffersgtexture.h"

// Mir
#include <mir/graphics/buffer.h>
#include <mir/geometry/size.h>

namespace mg = mir::geometry;

MirBufferSGTexture::MirBufferSGTexture(std::shared_ptr<mir::graphics::Buffer> buffer)
    : QSGTexture()
    , m_textureId(0)
{
    glGenTextures(1, &m_textureId);

    setFiltering(QSGTexture::Linear);
    setHorizontalWrapMode(QSGTexture::ClampToEdge);
    setVerticalWrapMode(QSGTexture::ClampToEdge);

    setBuffer(buffer);
}

MirBufferSGTexture::~MirBufferSGTexture()
{
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
    }
}

void MirBufferSGTexture::freeBuffer()
{
    m_mirBuffer.reset();
}

bool MirBufferSGTexture::hasBuffer() const
{
    return !!m_mirBuffer;
}

void MirBufferSGTexture::setBuffer(std::shared_ptr<mir::graphics::Buffer> buffer)
{
    m_mirBuffer = buffer;
    mg::Size size = buffer->size();
    m_height = size.height.as_int();
    m_width = size.width.as_int();
}

int MirBufferSGTexture::textureId() const
{
    return m_textureId;
}

QSize MirBufferSGTexture::textureSize() const
{
    /*
     * In between buffer changes we like to keep the same texture allocated,
     * for performance. So we need to remember the dimensions even after
     * freeBuffer(), to avoid asking the GPU to reallocate texture memory.
     */
    return QSize(m_width, m_height);
}

bool MirBufferSGTexture::hasAlphaChannel() const
{
    if (!m_mirBuffer) {
        return false;
    }

    // TODO: Support new pixel formats.
    // TODO more: Add a public function to do this in Mir.
    return m_mirBuffer->pixel_format() == mir_pixel_format_abgr_8888
        || m_mirBuffer->pixel_format() == mir_pixel_format_argb_8888;
}

void MirBufferSGTexture::bind()
{
    if (m_mirBuffer) {
        /*
         * NOTE: Optimal usage of texture binding depends on the type of
         *       buffer being used. If it's a pure software buffer then
         *       gl_bind_to_texture() does a copy and you can discard the
         *       buffer immediately. For hardware buffers however it's more
         *       common that the texture shares memory with the buffer and
         *       so both must remain untouched till after swap buffers at
         *       least.
         */
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        updateBindOptions(true/* force */);
        m_mirBuffer->gl_bind_to_texture();
    } else {
        /*
         * Occasionally we all forget to set a buffer. However it's most
         * helpful to see such mistakes as a black surface, rather than crash.
         */
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
