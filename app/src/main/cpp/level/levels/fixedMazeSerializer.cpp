/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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
#include "fixedMaze.hpp"
#include "../../serializeSaveDataInternals.hpp"

void to_json(nlohmann::json &j, FixedMazeSaveData const &val) {
    to_json(j, boost::implicit_cast<LevelSaveData const &>(val));
}

void from_json(nlohmann::json const &j, FixedMazeSaveData &val) {
    from_json(j, boost::implicit_cast<LevelSaveData&>(val));
}

Level::SaveLevelDataFcn FixedMaze::getSaveLevelDataFcn() {
    auto sd = std::make_shared<FixedMazeSaveData>();
    return {[sd](std::shared_ptr<GameSaveData> gsd) -> std::vector<uint8_t> {
        return saveGameData(gsd, sd);
    }};
}
