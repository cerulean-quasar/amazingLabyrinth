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

#include "renderDetailsGL.hpp"

namespace shadowsChaining {
    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(
                parameters.useIntTexture, parameters.width, parameters.height);

        std::vector<graphicsGL::Framebuffer::ColorImageFormat> colorImageFormats;
        // for shadow mapping.
        if (rd->m_useIntTexture) {
            colorImageFormats.emplace_back(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        } else {
            colorImageFormats.emplace_back(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }
        rd->m_framebufferShadows = std::make_shared<graphicsGL::Framebuffer>(
                rd->m_surfaceWidth, rd->m_surfaceHeight, colorImageFormats);

        auto refShadows = renderLoader->load(
                gameRequester, shadows::RenderDetailsGL::name(), parameters);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters.width;
        parametersWithShadows.height = parameters.height;
        parametersWithShadows.useIntTexture = parameters.useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, objectWithShadows::RenderDetailsGL::name(), parametersWithShadows);

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rd), refObjectWithShadows, refShadows);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.")
        }

        auto refShadows = renderLoader->load(
                gameRequester, renderLoader, shadows::RenderDetailsGL::name(), parameters);

        renderDetails::ParametersWithShadowsGL parametersWithShadows = {};
        parametersWithShadows.width = parameters.width;
        parametersWithShadows.height = parameters.height;
        parametersWithShadows.useIntTexture = parameters.useIntTexture;
        parametersWithShadows.shadowsFB = rd->m_framebufferShadows;

        auto refObjectWithShadows = renderLoader->load(
                gameRequester, renderLoader, objectWithShadows::RenderDetailsGL::name(),
                parametersWithShadows);

        rd->m_shadowsRenderDetails = refShadows.renderDetails;
        rd->m_objectWithShadowsRenderDetails = refObjectWithShadows.renderDetails;

        return createReference(std::move(rd), refObjectWithShadows, refShadows);
    }
}