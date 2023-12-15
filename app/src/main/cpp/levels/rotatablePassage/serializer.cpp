/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#include "../basic/serializer.hpp"
#include "../basic/level.hpp"
#include "../../levelTracker/internals.hpp"
#include "../movablePassageAlgorithmsSerializer.hpp"

#include "loadData.hpp"
#include "level.hpp"
#include "serializer.hpp"

namespace rotatablePassage {
    char constexpr const *ObjectReference = "ObjReference";
    char constexpr const *ComponentType = "ComponentType";
    char constexpr const *Nbr90DegreeRotations = "Nbr90DegreeRotations";
    void to_json(nlohmann::json &j, PlacementSaveData const &val) {
        j[ObjectReference] = val.objReference;
        j[ComponentType] = val.componentType;
        j[Nbr90DegreeRotations] = val.nbr90DegreeRotations;
    }

    char constexpr const *GameBoardPlacements = "GameBoardPlacements";
    char constexpr const *PathLockedInPlace = "PathLockedInPlace";
    char constexpr const *BallStartRC = "BallStartRC";
    char constexpr const *BallEndRC = "BallEndRC";
    char constexpr const *BallRC = "BallRC";
    char constexpr const *BallPosition = "BallPosition";
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
        j[GameBoardPlacements] = val.gameBoardPlacements;
        j[PathLockedInPlace] = val.pathLockedInPlace;
        j[BallStartRC] = val.ballStartRC;
        j[BallEndRC] = val.ballEndRC;
        j[BallRC] = val.ballRC;
        j[BallPosition] = val.ballPosition;
    }

    void from_json(nlohmann::json const &j, PlacementSaveData &val) {
        val.objReference = j[ObjectReference].get<ObjReference>();
        val.componentType = j[ComponentType].get<Component::ComponentType>();
        val.nbr90DegreeRotations = j[Nbr90DegreeRotations].get<uint32_t>();
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
        val.gameBoardPlacements = j[GameBoardPlacements].get<std::vector<std::vector<PlacementSaveData>>>();
        val.pathLockedInPlace = j[PathLockedInPlace].get<std::vector<Point<uint32_t>>>();
        val.ballStartRC = j[BallStartRC].get<Point<uint32_t>>();
        val.ballEndRC = j[BallEndRC].get<Point<uint32_t>>();
        val.ballRC = j[BallRC].get<Point<uint32_t>>();
        val.ballPosition = j[BallPosition].get<Point<float>>();
    }

    char constexpr const *NumberRows = "NumberRows";
    char constexpr const *DfsSearch = "DfsSearch";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[NumberRows] = val.numberRows;
        j[DfsSearch] = val.dfsSearch;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.numberRows = j[NumberRows].get<uint32_t>();
        val.dfsSearch = j[DfsSearch].get<bool>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        auto sd = std::make_shared<LevelSaveData>();
        sd->m_version = levelSaveDataVersion;

        for (size_t rowIndex = 0; rowIndex < m_gameBoard.heightInTiles(); rowIndex++) {
            std::vector<PlacementSaveData> row;
            for (size_t colIndex = 0; colIndex < m_gameBoard.widthInTiles(); colIndex++) {
                auto &b = m_gameBoard.block(rowIndex, colIndex);
                auto &p = b.component()->placement(b.placementIndex());

                auto ref = p.objReference();
                if (ref == boost::none) {
                    ref.reset(ObjReference());
                }
                row.emplace_back(ref.get(), b.component()->type(), p.nbr90DegreeRotations());
            }
            sd->gameBoardPlacements.push_back(row);
        }

        sd->pathLockedInPlace = pathLockedInPlace(m_gameBoard, m_ballStartRow, m_ballStartCol);
        sd->ballStartRC = Point<uint32_t>(m_ballStartRow, m_ballStartCol);
        sd->ballEndRC = Point<uint32_t>(m_endRow, m_endCol);
        sd->ballRC = Point<uint32_t>(m_ballRow, m_ballCol);
        sd->ballPosition = Point<float>(m_ball.position.x, m_ball.position.y);

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = *sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace rotatablePassage