/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_DARK_MAZE_LEVEL_HPP
#define AMAZING_LABYRINTH_DARK_MAZE_LEVEL_HPP

#include "../openAreaMaze/level.hpp"

namespace darkMaze {
    class Level : public openAreaMaze::Level {
    public:
        static char constexpr const *m_name = "darkMaze";

        /** Use darkv2 as the default render details, and standard as the render details for the
         * hole and ball.  The floor is using the default render details.
         */
        struct Request : public openAreaMaze::Level::Request {
            Request(levelDrawer::Adaptor levelDrawer, bool shadowsEnabled)
                    : openAreaMaze::Level::Request(std::move(levelDrawer), shadowsEnabled)
            {}

            void defaultRD() override {
                renderDetails::Query query{
                    renderDetails::DrawingStyle::darkV2,
                    {renderDetails::Features::texture,
                        renderDetails::Features::color},
                    {}};

                m_levelDrawer.requestRenderDetails(query, getParameters());
            }

            levelDrawer::DrawObjReference addBall(
                std::shared_ptr<levelDrawer::ModelDescription> const &obj,
                std::shared_ptr<levelDrawer::TextureDescription> const &tx) override
            {
                renderDetails::Query query{
                        renderDetails::DrawingStyle::standard,
                        {renderDetails::Features::texture,
                         renderDetails::Features::color},
                        {}};

                return m_levelDrawer.addObject(
                    obj, tx,
                    query, levelDrawer::DefaultConfig::getDefaultParameters());
            }

            levelDrawer::DrawObjReference addHole(
                std::shared_ptr<levelDrawer::ModelDescription> const &obj,
                std::shared_ptr<levelDrawer::TextureDescription> const &tx) override
            {
                renderDetails::Query query{
                        renderDetails::DrawingStyle::standard,
                        {renderDetails::Features::texture,
                         renderDetails::Features::color},
                        {}};

                return m_levelDrawer.addObject(
                    obj, tx,
                    query, levelDrawer::DefaultConfig::getDefaultParameters());
            }

            static std::shared_ptr<renderDetails::ParametersPerspective> getParameters() {
                auto parameters = levelDrawer::DefaultConfig::getDefaultParameters();
                parameters->lightingSources.push_back(glm::vec3{0.0, 0.0, 0.0});

                return parameters;
            }
        };

        char const *name() override { return m_name; }

        bool updateDrawObjects() override;

        bool checkFinishCondition(float) override;

        Level(levelDrawer::Adaptor inLevelDrawer,
                std::shared_ptr<generatedMaze::LevelConfigData> const &lcd,
                std::shared_ptr<generatedMaze::LevelSaveData> const &sd,
                float maxZ,
                Request &request)
                : openAreaMaze::Level(
                        std::move(inLevelDrawer), lcd, sd, maxZ, request)
        {
            auto parameters = Request::getParameters();
            m_parameters = *parameters;
            m_parameters.lightingSources[0] = m_ball.position;
            m_parameters.lightingSources[1] = getCellCenterPosition(m_mazeBoard.rowEnd(), m_mazeBoard.colEnd());
            m_parameters.lightingSources[1].z = m_parameters.lightingSources[0].z;
            m_levelDrawer.updateCommonObjectData(m_objRefsWalls[0], m_parameters);
        }

        ~Level() override = default;
    private:
        renderDetails::ParametersPerspective m_parameters;
    };
} // namespace darkMaze
#endif // AMAZING_LABYRINTH_DARK_MAZE_LEVEL_HPP
