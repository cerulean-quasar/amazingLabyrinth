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
#include <json.hpp>

#include <boost/implicit_cast.hpp>
#include "../../levelTracker/internals.hpp"
#include "../basic/serializer.hpp"
#include "level.hpp"
#include "loadData.hpp"

namespace starter{
    char constexpr const *StartUpMessages = "StartupMessages";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelConfigData const &>(val));
        j[StartUpMessages] = val.startupMessages;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        from_json(j, boost::implicit_cast<basic::LevelConfigData &>(val));
        val.startupMessages = j[StartUpMessages].get<std::vector<std::string>>();
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::starterTable, LevelConfigData, LevelSaveData, Level> registerLevel;
}