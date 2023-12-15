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

#ifndef AMAZING_LABYRINTH_BASIC_SERIALIZER_HPP
#define AMAZING_LABYRINTH_BASIC_SERIALIZER_HPP

#include <memory>
#include <json.hpp>

#include "loadData.hpp"

namespace basic {
    void to_json(nlohmann::json &j, LevelSaveData const &val);
    void from_json(nlohmann::json const &j, LevelSaveData &val);

    void to_json(nlohmann::json &j, LevelConfigData const &val);
    void from_json(nlohmann::json const &j, LevelConfigData &val);
} // namespace basic

#endif // AMAZING_LABYRINTH_BASIC_SERIALIZER_HPP
