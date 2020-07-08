/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of AmazingLabyrinth.
 *
 *  AmazingLabyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AmazingLabyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AmazingLabyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <memory>
#include <vector>

#include "../renderDetailsGL.hpp"
#include "renderDetailsDataGL.hpp"
#include "../shadows/renderDetailsDataGL.hpp"

namespace textureWithShadows {
    RenderDetailsDataGL::RenderDetailsDataGL(std::shared_ptr<GameRequester> const &inGameRequester,
                                             bool useIntTexture,
                                             uint32_t inWidth, uint32_t inHeight)
            : RenderDetailsData(inWidth, inHeight),
            m_useIntTexture{useIntTexture},
            m_framebufferShadows{},
            m_shadowsRenderDetails{std::make_shared<shadows::RenderDetailsDataGL>(inGameRequester, inWidth, inHeight)},
            m_mainProgramID{renderDetails::loadShaders(inGameRequester, SHADER_VERT_FILE, SHADER_FRAG_FILE)}
    {
        std::vector<renderDetails::Framebuffer::ColorImageFormat> colorImageFormats;
        if (m_useIntTexture) {
            colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        } else {
            colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        m_framebufferShadows = std::make_shared<renderDetails::Framebuffer>(
                m_surfaceWidth, m_surfaceHeight, colorImageFormats);
    }
}
