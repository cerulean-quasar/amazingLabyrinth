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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP

#include <functional>
#include <unordered_map>

#include <json.hpp>

#include "../levels/basic/level.hpp"
#include "../levels/finisher/types.hpp"
#include "levelTracker.hpp"

void to_json(nlohmann::json &j, Point<uint32_t> const &val);
void from_json(nlohmann::json const &j, Point<uint32_t> &val);
void to_json(nlohmann::json &j, Point<float> const &val);
void from_json(nlohmann::json const &j, Point<float> &val);

namespace levelTracker {
    namespace DataVariables {
        char constexpr const *GameSaveDataVersion = "GameSaveDataVersion";
        char constexpr const *GameSaveDataLevelName = "LevelName";
        char constexpr const *GameSaveDataScreenSize = "ScreenSize";
        char constexpr const *GameSaveDataLevel = "LevelSaveData";
        char constexpr const *GameSaveDataNeedsStarter = "LevelNeedsStarter";
    }

    using GenerateLevelGeneratorFcn = std::function<GenerateLevelFcn(nlohmann::json const &,
            nlohmann::json const *, float, bool)>;
    using LevelMapEntry = std::pair<std::string, GenerateLevelGeneratorFcn>;
    using LevelMapTable = std::unordered_map<std::string, GenerateLevelGeneratorFcn>;

    using GenerateFinisherGeneratorFcn = std::function<GenerateFinisherFcn(nlohmann::json const &, float)>;
    using FinisherMapEntry = std::pair<std::string, GenerateFinisherGeneratorFcn>;
    using FinisherMapTable = std::unordered_map<std::string, GenerateFinisherGeneratorFcn>;

    LevelMapTable &starterTable();

    LevelMapTable &levelTable();

    FinisherMapTable &finisherTable();

    template<typename Table, Table &(*table)(), typename LevelConfigDataType, typename LevelSaveDataType, typename LevelType>
    class Register {
    public:
        Register() {
            throw std::runtime_error("Unknown table type for registering level");
        }
    };

    template<typename LevelConfigDataType, typename LevelSaveDataType, typename LevelType>
    class Register<LevelMapTable, levelTable, LevelConfigDataType, LevelSaveDataType, LevelType> {
    public:
        Register() {
            levelTable().insert(std::make_pair(LevelType::m_name,
                 GenerateLevelGeneratorFcn(
                     [](nlohmann::json const &lcdjson, nlohmann::json const *sdjson, float z, bool enableShadows) -> levelTracker::GenerateLevelFcn
                     {
                         auto lcd = std::make_shared<LevelConfigDataType>();
                         *lcd = lcdjson.get<LevelConfigDataType>();
                         std::shared_ptr<LevelSaveDataType> sd;
                         if (sdjson) {
                             sd = std::make_shared<LevelSaveDataType>();
                             *sd = sdjson->get<LevelSaveDataType>();
                         }
                         return GenerateLevelFcn(
                             [lcd, sd, z, enableShadows](levelDrawer::Adaptor inLevelDrawer) -> std::shared_ptr<basic::Level>
                             {
                                 typename LevelType::Request request(inLevelDrawer, enableShadows);
                                 return std::make_shared<LevelType>(
                                         std::move(inLevelDrawer), lcd, sd, z, request);
                             });
                     }))
            );
        }
    };

    template<typename LevelConfigDataType, typename LevelSaveDataType, typename LevelType>
    class Register<LevelMapTable, starterTable, LevelConfigDataType, LevelSaveDataType, LevelType> {
    public:
        Register() {
            starterTable().insert(std::make_pair(LevelType::m_name,
            GenerateLevelGeneratorFcn(
            [](nlohmann::json const &lcdjson, nlohmann::json const *, float z, bool enableShadows) -> levelTracker::GenerateLevelFcn
            {
                auto lcd = std::make_shared<LevelConfigDataType>();
                *lcd = lcdjson.get<LevelConfigDataType>();
                return GenerateLevelFcn(
                    [lcd, sd(std::shared_ptr<LevelSaveDataType>()), z, enableShadows](
                            levelDrawer::Adaptor inLevelDrawer) -> std::shared_ptr<basic::Level>
                    {
                        typename LevelType::Request request(inLevelDrawer, enableShadows);
                        return std::make_shared<LevelType>(
                                std::move(inLevelDrawer), lcd, sd, z, request);
                    });
            }))
            );
        }
    };

    template<typename LevelConfigDataType, typename LevelType>
    class Register<FinisherMapTable, finisherTable, LevelConfigDataType, void, LevelType> {
    public:
        Register() {
            finisherTable().insert(std::make_pair(LevelType::m_name,
                 GenerateFinisherGeneratorFcn(
                     [](nlohmann::json const &lcdjson, float finisherZ) -> levelTracker::GenerateFinisherFcn
                     {
                         auto lcd = std::make_shared<LevelConfigDataType>(lcdjson.get<LevelConfigDataType>());
                         return levelTracker::GenerateFinisherFcn(
                             [lcd{std::move(lcd)}, finisherZ](levelDrawer::Adaptor inLevelDrawer,
                                     float centerX, float centerY, float centerZ)
                                     -> std::shared_ptr<finisher::LevelFinisher>
                             {
                                 return std::make_shared<LevelType>(
                                         std::move(inLevelDrawer), lcd,
                                         centerX, centerY, centerZ, finisherZ);
                             });
                     }))
            );
        }
    };

    void to_json(nlohmann::json &j, GameSaveData const &gsd);
    void from_json(nlohmann::json const &j, GameSaveData &gsd);
}

#endif //AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP
