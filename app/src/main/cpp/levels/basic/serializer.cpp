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

    char constexpr const *ModelName = "Name";
    char constexpr const *ModelType = "Type";
    char constexpr const *ModelFiles = "Models";
    char constexpr const *Textures = "Textures";
    char constexpr const *AlternateTextures = "AlternateTextures";
    char constexpr const *DefaultColor = "DefaultColor";
    char constexpr const *LoadVertexNormals = "LoadVertexNormals";
    char constexpr const *LoadFaceNormals = "LoadFaceNormals";

    void to_json(nlohmann::json &j, ModelConfigData const &val) {
        j[ModelName] = val.modelName;
        if (!val.modelFiles.empty()) {
            j[ModelFiles] = val.modelFiles;
        } else {
            j[ModelType] = val.modelType;
        }
        if (!val.textures.empty()) {
            j[Textures] = val.textures;
        }
        if (!val.alternateTextures.empty()) {
            j[AlternateTextures] = val.alternateTextures;
        }
        j[DefaultColor] = val.defaultColor;
        j[LoadFaceNormals] = val.loadFaceNormals;
        j[LoadVertexNormals] = val.loadVertexNormals;
    }

    void from_json(nlohmann::json const &j, ModelConfigData &val) {
        val = {};
        val.modelName = j[ModelName];
        if (j.contains(ModelFiles)) {
            val.modelFiles = j[ModelFiles].get<std::vector<std::string>>();
            val.modelType = 0;
        } else {
            val.modelType = j[ModelType];
        }

        if (j.contains(Textures)) {
            val.textures = j[Textures].get<std::vector<std::string>>();
        }
        if (j.contains(AlternateTextures)) {
            val.alternateTextures = j[AlternateTextures].get<std::vector<std::string>>();
        }
        if (j.contains(DefaultColor)) {
            val.defaultColor = j[DefaultColor];
        } else {
            val.defaultColor = defaultModelColor;
        }

        if (j.contains(LoadFaceNormals)) {
            val.loadFaceNormals = j[LoadFaceNormals];
        } else {
            val.loadFaceNormals = true;
        }
        if (j.contains(LoadVertexNormals)) {
            val.loadVertexNormals = j[LoadVertexNormals];
        } else {
            val.loadVertexNormals = false;
        }
    }

    static char constexpr const *Models = "ModelConfigs";

    char constexpr const *BallDiagonalRatio = "BallDiagonalRatio";
    char constexpr const *BounceEnabled = "BounceEnabled";
    void to_json(nlohmann::json &j, LevelConfigData const &val) {
        j[Models] = val.models;
        j[BounceEnabled] = val.bounceEnabled;
        j[BallDiagonalRatio] = val.ballSizeDiagonalRatio;
    }

    void from_json(nlohmann::json const &j, LevelConfigData &val) {
        val.models = j[Models].get<std::vector<ModelConfigData>>();
        val.bounceEnabled = j[BounceEnabled].get<bool>();
        val.ballSizeDiagonalRatio = j[BallDiagonalRatio].get<float>();
    }
} // namespace basic