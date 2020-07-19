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
#include "levelDrawerGL.hpp"
#include "levelDrawerGraphics.hpp"

namespace levelDrawer {
    template <>
    void LevelDrawerGraphics<LevelDrawerGLTraits>::draw(
            LevelDrawerGLTraits::DrawArgumentType info)
    {
        auto rulesList = getDrawRules();

        // add the pre main draw commands to the command buffer.
        for (auto const &rule : rulesList) {
            rule.renderDetails->preMainDraw(0, rule.commonObjectDataList,
                    m_drawObjectTableList, rule.indicesPerLevelType);
        }

        // The clear background color
        glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a);
        checkGraphicsError();

        // The clear depth buffer
        glClearDepthf(1.0f);
        checkGraphicsError();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGraphicsError();

        // add the commands to the command buffer for the main draw.
        for (auto index : std::vector<ObjectType>{LEVEL, STARTER, FINISHER}) {
            for (auto const &rule : rulesList) {
                rule.renderDetails->draw(
                        0, rule.commonObjectDataList[index], m_drawObjectTableList[index],
                        rule.indicesPerLevelType[index]);
            }
        }
    }

    template <>
    LevelDrawerGraphics<LevelDrawerGLTraits>::LevelDrawerGraphics(
            LevelDrawerGLTraits::NeededForDrawingType /* no extra members needed for drawing */,
            std::shared_ptr<LevelDrawerGLTraits::RenderLoaderType> inRenderLoader,
            std::shared_ptr<GameRequester> inGameRequester)
            : m_modelTable{},
            m_textureTable{},
            m_drawObjectTableList{
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>(),
                std::make_shared<LevelDrawerGLTraits::DrawObjectTableType>() },
            m_renderLoader{std::move(inRenderLoader)},
            m_gameRequester{std::move(inGameRequester)}
    {}
}