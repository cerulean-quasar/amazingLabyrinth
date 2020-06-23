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
#include <json.hpp>
#include <boost/implicit_cast.hpp>

#include "../../levelTracker/internals.hpp"
#include "../basic/serializer.hpp"

#include "level.hpp"
#include "serializer.hpp"

namespace movingSafeAreas {
    char constexpr const *BallLocation = "BallLocation";
    char constexpr const *QuadRows = "QuadRows";
    char constexpr const *Positions = "Positions";
    char constexpr const *Speed = "Speed";
    char constexpr const *Scale = "Scale";

    void to_json(nlohmann::json &j, QuadRowSaveData const &val) {
        j[Positions] = val.positions;
        j[Speed] = val.speed;
        j[Scale] = val.scale;
    }

    void from_json(nlohmann::json const &j, QuadRowSaveData &val) {
        val.positions = j[Positions].get<std::vector<Point<float>>>();
        val.speed = j[Speed].get<float>();
        val.scale = j[Scale].get<Point<float>>();
    }

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
        j[BallLocation] = val.ball;
        j[QuadRows] = val.quadRows;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
        val.ball = j[BallLocation].get<Point<float>>();
        val.quadRows = j[QuadRows].get<std::vector<QuadRowSaveData>>();
    }

    char constexpr const *StartQuadTexture = "StartQuadTexture";
    char constexpr const *EndQuadTexture = "EndQuadTexture";
    char constexpr const *MiddleQuadTextures = "MiddleQuadTextures";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[StartQuadTexture] = val.startQuadTexture;
        j[EndQuadTexture] = val.endQuadTexture;
        j[MiddleQuadTextures] = val.middleQuadTextures;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.startQuadTexture = j[StartQuadTexture].get<std::string>();
        val.endQuadTexture = j[EndQuadTexture].get<std::string>();
        val.middleQuadTextures = j[MiddleQuadTextures].get<std::vector<std::string>>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        std::vector<QuadRowSaveData> quadRows;
        quadRows.reserve(m_movingQuads.size());
        for (auto const &movingQuad : m_movingQuads) {
            std::vector<Point<float>> positions;
            for (auto const &position : movingQuad.positions) {
                positions.emplace_back(position.x, position.y);
            }
            quadRows.emplace_back(std::move(positions), movingQuad.speed,
                                  Point<float>{movingQuad.scale.x, movingQuad.scale.y});
        }
        LevelSaveData sd(
                Point<float>{m_ball.position.x, m_ball.position.y},
                std::move(quadRows));
        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace movingSafeAreas