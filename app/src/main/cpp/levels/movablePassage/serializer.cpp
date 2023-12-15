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

#include "../../levelTracker/internals.hpp"
#include "../basic/serializer.hpp"

#include "../movablePassageAlgorithmsSerializer.hpp"
#include "serializer.hpp"
#include "level.hpp"
#include "loadData.hpp"
#include "../basic/level.hpp"

namespace movablePassage {
    char constexpr const *StraightPositions = "StraightPositions";
    char constexpr const *TJunctionPositions = "TJunctionPositions";
    char constexpr const *CrossJunctionPositions = "CrossJunctionPositions";
    char constexpr const *TurnPositions = "TurnPositions";
    char constexpr const *BallRowColumn = "BallRowColumn";
    char constexpr const *PathLockedInPlace = "PathLockedInPlace";
    char constexpr const *PlacementPosition = "PlacementPosition";
    char constexpr const *Nbr90DegreeRotations = "Nbr90DegreeRotations";
    char constexpr const *BallPosition = "BallPosition";
    char constexpr const *GameBoardObjReferences = "GameBoardObjReferences";

    void to_json(nlohmann::json &j, PlacementSaveData const &val) {
        j[PlacementPosition] = val.rowCol;
        j[Nbr90DegreeRotations] = val.nbr90DegreeRotations;
    }

    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));

        j[StraightPositions] = val.straightPositions;
        j[TJunctionPositions] = val.tjunctionPositions;
        j[CrossJunctionPositions] = val.crossjunctionPositions;
        j[TurnPositions] = val.turnPositions;
        j[PathLockedInPlace] = val.pathLockedInPlace;
        j[BallRowColumn] = val.ballRC;
        j[BallPosition] = val.ballPosition;
        j[GameBoardObjReferences] = val.gameBoardObjReferences;
    }

    void from_json(nlohmann::json const &j, PlacementSaveData &val) {
        val.rowCol = j[PlacementPosition].get<Point<uint32_t>>();
        val.nbr90DegreeRotations = j[Nbr90DegreeRotations].get<uint32_t>();
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
        val.straightPositions = j[StraightPositions].get<std::vector<PlacementSaveData>>();
        val.tjunctionPositions = j[TJunctionPositions].get<std::vector<PlacementSaveData>>();
        val.crossjunctionPositions = j[CrossJunctionPositions].get<std::vector<PlacementSaveData>>();
        val.turnPositions = j[TurnPositions].get<std::vector<PlacementSaveData>>();
        val.pathLockedInPlace = j[PathLockedInPlace].get<std::vector<Point<uint32_t>>>();
        val.ballRC = j[BallRowColumn].get<Point<uint32_t>>();
        val.ballPosition = j[BallPosition].get<Point<float>>();
        val.gameBoardObjReferences = j[GameBoardObjReferences].get<std::vector<std::vector<ObjReference>>>();
    }

    char constexpr const *Row = "Row";
    char constexpr const *Col = "Col";
    void to_json(nlohmann::json &j, RockPlacement const &val) {
        j[Row] = val.row;
        j[Col] = val.col;
    }

    void from_json(nlohmann::json const &j, RockPlacement &val) {
        val.row = j[Row].get<uint32_t>();
        val.col = j[Col].get<uint32_t>();
    }

    char constexpr const *Straight = "Straight";
    char constexpr const *Turn = "Turn";
    char constexpr const *CrossJunction = "CrossJunction";
    char constexpr const *TJunction = "TJunction";
    char constexpr const *NumberPlacementsStr = "NumberPlacements";
    void to_json(nlohmann::json &j, NumberPlacements const &val) {
        j[Straight] = val.straight;
        j[Turn] = val.turn;
        j[CrossJunction] = val.crossJunction;
        j[TJunction] = val.tJunction;
    }

    void from_json(nlohmann::json const &j, NumberPlacements &val) {
        val.straight = j[Straight].get<uint32_t>();
        val.turn = j[Turn].get<uint32_t>();
        val.crossJunction = j[CrossJunction].get<uint32_t>();
        val.tJunction = j[TJunction].get<uint32_t>();
    }

    char constexpr const *NumberTilesX = "NumberTilesX";
    char constexpr const *NumberTilesY = "NumberTilesY";
    char constexpr const *StartColumn = "StartColumn";
    char constexpr const *EndColumn = "EndColumn";
    char constexpr const *RockPlacements = "RockPlacements";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));

        j[NumberTilesX] = val.numberTilesX;
        j[NumberTilesY] = val.numberTilesY;
        j[StartColumn] = val.startColumn;
        j[EndColumn] = val.endColumn;
        j[NumberPlacementsStr] = val.nbrPlacements;
        j[RockPlacements] = val.rockPlacements;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));

        val.numberTilesX = j[NumberTilesX].get<uint32_t>();
        val.numberTilesY = j[NumberTilesY].get<uint32_t>();
        val.startColumn = j[StartColumn].get<uint32_t>();
        val.endColumn = j[EndColumn].get<uint32_t>();
        val.nbrPlacements = j[NumberPlacementsStr].get<NumberPlacements>();
        val.rockPlacements = j[RockPlacements].get<std::vector<RockPlacement>>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        auto sd = std::make_shared<LevelSaveData>();

        // the component positions.
        auto initComponentVector =
            [&] (Component::ComponentType componentType, std::vector<PlacementSaveData> &vec) {
                for (auto placementIt = m_components[componentType]->placementsBegin();
                     placementIt != m_components[componentType]->placementsEnd();
                     placementIt++)
                {
                    vec.emplace_back(placementIt->row(), placementIt->col(),
                            placementIt->nbr90DegreeRotations());
                }
            };
        initComponentVector(Component::ComponentType::straight, sd->straightPositions);
        initComponentVector(Component::ComponentType::tjunction, sd->tjunctionPositions);
        initComponentVector(Component::ComponentType::crossjunction, sd->crossjunctionPositions);
        initComponentVector(Component::ComponentType::turn, sd->turnPositions);

        // the ball row and column
        sd->ballRC = Point<uint32_t>(m_ballRow, m_ballCol);

        // the ball position from the origin in x and y directions.
        sd->ballPosition = Point<float>(m_ball.position.x, m_ball.position.y);

        // the path the ball has traveled in user placeable components.
        sd->pathLockedInPlace = pathLockedInPlace(m_gameBoard, m_ballFirstPlaceableComponent.first-1,
                m_ballFirstPlaceableComponent.second);

        // the model/texture each component is using.
        for (size_t row = 0; row < m_gameBoard.heightInTiles() - m_nbrTileRowsForEnd; row++) {
            std::vector<ObjReference> refs;
            for (size_t col = 0; col < m_gameBoard.widthInTiles(); col++) {
                auto &b = m_gameBoard.block(row, col);
                auto ref = b.component()->placement(b.placementIndex()).objReference();
                if (ref == boost::none) {
                    // should mostly never happen, but just in case we are shutting down before
                    // updateStaticDrawObjects or updateDynamicDrawObjects happens...
                    refs.emplace_back();
                } else {
                    refs.emplace_back(ref.get());
                }
            }
            sd->gameBoardObjReferences.push_back(refs);
        }

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = *sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace movablePassage