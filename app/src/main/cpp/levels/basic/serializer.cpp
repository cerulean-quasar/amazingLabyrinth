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
#include "level.hpp"
#include "loadData.hpp"
#include "serializer.hpp"
#include "../../levelTracker/types.hpp"

namespace basic {
    char constexpr const *LevelVersion = "LevelVersion";
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        j[LevelVersion] = val.m_version;
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        val.m_version = j[LevelVersion].get<int>();
    }

    char constexpr const *BallDiagonalRatio = "BallDiagonalRatio";
    char constexpr const *BallModel = "BallModel";
    char constexpr const *BallTexture = "BallTexture";
    char constexpr const *BounceEnabled = "BounceEnabled";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        j[BallModel] = val.ballModel;
        j[BallTexture] = val.ballTexture;
        j[BounceEnabled] = val.bounceEnabled;
        j[BallDiagonalRatio] = val.ballSizeDiagonalRatio;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        val.ballModel = j[BallModel].get<std::string>();
        val.ballTexture = j[BallTexture].get<std::string>();
        val.bounceEnabled = j[BounceEnabled].get<bool>();
        val.ballSizeDiagonalRatio = j[BallDiagonalRatio].get<float>();
    }
} // namespace basic