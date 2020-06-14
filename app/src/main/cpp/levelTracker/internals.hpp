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

void to_json(nlohmann::json &j, Point<uint32_t> const &val);
void from_json(nlohmann::json const &j, Point<uint32_t> &val);
void to_json(nlohmann::json &j, Point<float> const &val);
void from_json(nlohmann::json const &j, Point<float> &val);

namespace levelTracker {
    using GenerateLevelFcn = std::function<std::shared_ptr<Level>(std::shared_ptr<GameRequester>,
            glm::mat4 const &, glm::mat4 const &, float)>;
    using LevelMapEntry = std::pair<std::string, GenerateLevelFcn>;
    using LevelMapTable = std::unordered_map<std::string, GenerateLevelFcn>;

    using GenerateFinisherFcn = std::function<std::shared_ptr<LevelFinish>(std::shared_ptr<GameRequester>,
            glm::mat4 const &, glm::mat4 const &, float)>;
    using FinisherMapEntry = std::pair<std::string, GenerateFinisherFcn>;
    using FinisherMapTable = std::unordered_map<std::string, GenerateFinisherFcn>;

    LevelMapTable &starterTable();

    LevelMapTable &levelTable();

    FinisherMapTable &finisherTable();

    template<typename TableEntry, typename Table, Table (*table)()>
    class Register {
    public:
        Register(TableEntry entry) {
            table().insert(entry);
        }
    };

    using RegisterStarter = Register<LevelMapEntry, LevelMapTable, starterTable>;
    using RegisterLevel = Register<LevelMapEntry, LevelMapTable, levelTable>;
    using RegisterFinisher = Register<FinisherMapEntry, FinisherMapTable, finisherTable>;
}

#endif //AMAZING_LABYRINTH_LEVEL_TRACKER_INTERNALS_HPP
