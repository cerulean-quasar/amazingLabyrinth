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
#include "../../levelTracker/internals.hpp"
#include "types.hpp"
#include "loadData.hpp"

namespace manyQuadsCoverUp {
    char constexpr const *Textures = "Textures";
    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        val.textures = j[Textures].get<std::vector<std::string>>();
    }

    levelTracker::Register<levelTracker::FinisherMapTable, levelTracker::finisherTable, LevelConfigData, void, LevelFinisher> registerFinisher;
} // namespace manyQuadsCoverUp

namespace growingQuad {
    char constexpr const *Texture = "Texture";
    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        val.texture = j[Texture].get<std::string>();
    }

    levelTracker::Register<levelTracker::FinisherMapTable, levelTracker::finisherTable, LevelConfigData, void, LevelFinisher> registerFinisher;
} // growingQuad
