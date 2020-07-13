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
#include "renderDetailsGL.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"

namespace objectWithShadows {
    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(gameRequester, parameters.width,
                                                    parameters.height);

        renderDetails::ParametersWithShadowsGL const &p =
                dynamic_cast<renderDetails::ParametersWithShadowsGL const &>(parameters);

        auto cod = std::make_shared<CommonObjectDataGL>(p.shadowsFB,
                parameters.width / static_cast<float>(parameters.height), config);

        return createReference(std::move(rd), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        renderDetails::ParametersWithShadowsGL const &p =
                dynamic_cast<renderDetails::ParametersWithShadowsGL const &>(parameters);

        auto cod = std::make_shared<CommonObjectDataGL>(p.shadowsFB,
                parameters.width / static_cast<float>(parameters.height), config);

        return createReference(std::move(rdBase), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            std::shared_ptr<CommonObjectDataGL> cod)
    {
        renderDetails::ReferenceGL ref;
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData(
            [] (
                    std::shared_ptr<renderDetails::DrawObjectDataGL> const &,
                    std::shared_ptr<levelDrawer::TextureData>,
                    glm::mat4 modelMatrix) -> std::shared_ptr<renderDetails::DrawObjectDataGL>
            {
                return std::make_shared<DrawObjectDataGL>(std::move(modelMatrix));
            });
        ref.renderDetails = std::move(rd);
        ref.commonObjectData = std::move(cod);
        return std::move(ref);
    }

    RenderDetailsGL::RenderDetailsGL(
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth, uint32_t inHeight)
        : renderDetails::RenderDetailsGL(inWidth, inHeight),
        m_textureProgramID{loadShaders(inGameRequester, SHADER_VERT_FILE, TEXTURE_SHADER_FRAG_FILE)},
        m_colorProgramID{loadShaders(inGameRequester, SHADER_VERT_FILE, COLOR_SHADER_FRAG_FILE)}
    {}
}