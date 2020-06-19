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

#include "serializer.hpp"
#include "level.hpp"
#include "loadData.hpp"

namespace movablePassage {
    char constexpr const *StraightPositions = "StraightPositions";
    char constexpr const *TJunctionPositions = "TJunctionPositions";
    char constexpr const *CrossJunctionPositions = "CrossJunctionPositions";
    char constexpr const *TurnPositions = "TurnPositions";
    char constexpr const *BallRowColumn = "BallRowColumn";
    char constexpr const *PathLockedInPlace = "PathLockedInPlace";
    char constexpr const *PlacementPosition = "PlacementPosition";
    char constexpr const *Nbr90DegreeRotations = "Nbr90DegreeRotations";

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
        j[BallRowColumn] = val.ballRC;
        j[PathLockedInPlace] = val.pathLockedInPlace;
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
        val.ballRC = j[BallRowColumn].get<Point<uint32_t>>();
        val.pathLockedInPlace = j[PathLockedInPlace].get<std::vector<Point<uint32_t>>>();
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

    char constexpr const *Model = "Model";
    char constexpr const *Texture = "Texture";
    char constexpr const *NumberPlacements = "NumberPlacements";
    void to_json(nlohmann::json &j, ComponentConfig const &val) {
        j[Model] = val.model;
        j[Texture] = val.texture;
        j[NumberPlacements] = val.numberPlacements;
    }

    void from_json(nlohmann::json const &j, ComponentConfig &val) {
        val.model = j[Model].get<std::string>();
        val.texture = j[Texture].get<std::string>();
        val.numberPlacements = j[NumberPlacements].get<uint32_t>();
    }

    char constexpr const *NumberTilesX = "NumberTilesX";
    char constexpr const *NumberTilesY = "NumberTilesY";
    char constexpr const *StartColumn = "StartColumn";
    char constexpr const *EndColumn = "EndColumn";
    char constexpr const *EndTexture = "EndTexture";
    char constexpr const *EndOffBoardTexture = "EndOffBoardTexture";
    char constexpr const *PlacementLockedInPlaceTexture = "PlacementLockedInPlaceTexture";
    char constexpr const *RockModels = "RockModels";
    char constexpr const *RockTextures = "RockTextures";
    char constexpr const *DirtModels = "DirtModels";
    char constexpr const *DirtTextures = "DirtTextures";
    char constexpr const *BeginningSideModels = "BeginningSideModels";
    char constexpr const *BeginningSideTextures = "BeginningSideTextures";
    char constexpr const *BeginningOpenModels = "BeginningOpenModels";
    char constexpr const *BeginningOpenTextures = "BeginningOpenTextures";
    char constexpr const *BeginningCornerModels = "BeginningCornerModels";
    char constexpr const *BeginningCornerTextures = "BeginningCornerTextures";
    char constexpr const *Straight = "Straight";
    char constexpr const *Turn = "Turn";
    char constexpr const *CrossJunction = "CrossJunction";
    char constexpr const *TJunction = "TJunction";
    char constexpr const *RockPlacements = "RockPlacements";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));

        j[NumberTilesX] = val.numberTilesX;
        j[NumberTilesY] = val.numberTilesY;
        j[StartColumn] = val.startColumn;
        j[EndColumn] = val.endColumn;
        j[EndTexture] = val.endTexture;
        j[EndOffBoardTexture] = val.endOffBoardTexture;
        j[PlacementLockedInPlaceTexture] = val.placementLockedInPlaceTexture;
        j[RockModels] = val.rockModels;
        j[RockTextures] = val.rockTextures;
        j[DirtModels] = val.dirtModels;
        j[DirtTextures] = val.dirtTextures;

        j[BeginningSideModels] = val.beginningSideModels;
        j[BeginningSideTextures] = val.beginningSideTextures;

        j[BeginningOpenModels] = val.beginningOpenModels;
        j[BeginningOpenTextures] = val.beginningOpenTextures;

        j[BeginningCornerModels] = val.beginningCornerModels;
        j[BeginningCornerTextures] = val.beginningCornerTextures;

        j[Straight] = val.straight;
        j[Turn] = val.turn;
        j[CrossJunction] = val.crossjunction;
        j[TJunction] = val.tjunction;
        j[RockPlacements] = val.rockPlacements;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));

        val.numberTilesX = j[NumberTilesX].get<uint32_t>();
        val.numberTilesY = j[NumberTilesY].get<uint32_t>();
        val.startColumn = j[StartColumn].get<uint32_t>();
        val.endColumn = j[EndColumn].get<uint32_t>();
        val.endTexture = j[EndTexture].get<std::string>();
        val.endOffBoardTexture = j[EndOffBoardTexture].get<std::string>();
        val.placementLockedInPlaceTexture = j[PlacementLockedInPlaceTexture].get<std::string>();
        val.rockModels = j[RockModels].get<std::vector<std::string>>();
        val.rockTextures = j[RockTextures].get<std::vector<std::string>>();
        val.dirtModels = j[DirtModels].get<std::vector<std::string>>();
        val.dirtTextures = j[DirtTextures].get<std::vector<std::string>>();

        val.beginningSideModels = j[BeginningSideModels].get<std::vector<std::string>>();
        val.beginningSideTextures = j[BeginningSideTextures].get<std::vector<std::string>>();

        val.beginningOpenModels = j[BeginningOpenModels].get<std::vector<std::string>>();
        val.beginningOpenTextures = j[BeginningOpenTextures].get<std::vector<std::string>>();

        val.beginningCornerModels = j[BeginningCornerModels].get<std::vector<std::string>>();
        val.beginningCornerTextures = j[BeginningCornerTextures].get<std::vector<std::string>>();

        val.straight = j[Straight].get<ComponentConfig>();
        val.turn = j[Turn].get<ComponentConfig>();
        val.crossjunction = j[CrossJunction].get<ComponentConfig>();
        val.tjunction = j[TJunction].get<ComponentConfig>();
        val.rockPlacements = j[RockPlacements].get<std::vector<RockPlacement>>();
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                  char const *saveLevelDataKey) {
        auto sd = std::make_shared<LevelSaveData>();
        auto initComponentVector =
            [&] (Component::ComponentType componentType, std::vector<PlacementSaveData> &vec) {
                for (auto placementIt = m_components[componentType]->placementsBegin();
                     placementIt != m_components[componentType]->placementsEnd();
                     placementIt++)
                {
                    float angle = placementIt->rotationAngle();
                    float halfpi = glm::radians(90.0f);
                    uint32_t nbrRotations = 0;
                    while (angle > 0) {
                        nbrRotations ++;
                        angle -= halfpi;
                    }
                    vec.emplace_back(placementIt->row(), placementIt->col(), nbrRotations);
                }
            };
        initComponentVector(Component::ComponentType::straight, sd->straightPositions);
        initComponentVector(Component::ComponentType::tjunction, sd->tjunctionPositions);
        initComponentVector(Component::ComponentType::crossjunction, sd->crossjunctionPositions);
        initComponentVector(Component::ComponentType::turn, sd->turnPositions);
        sd->ballRC = Point<uint32_t>(m_ballRow, m_ballCol);

        bool done = false;
        Point<uint32_t> startRC{m_ballFirstPlaceableComponent.first, m_ballFirstPlaceableComponent.second};
        do {
            auto b = m_gameBoard.block(startRC.x, startRC.y);
            auto &placement = b.component()->placement(b.placementIndex());
            if (placement.prev().first != nullptr) {
                sd->pathLockedInPlace.emplace_back(startRC);
            } else {
                done = true;
            }
            if (placement.next().first != nullptr) {
                auto &nextPlacement = placement.next().first->placement(placement.next().second);
                startRC.x = nextPlacement.row();
                startRC.y = nextPlacement.col();
            } else {
                done = true;
            }
        } while (!done);

        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = *sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace movablePassage