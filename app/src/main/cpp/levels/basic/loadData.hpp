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

#ifndef AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
#include <string>
#include <array>

namespace basic {
    uint8_t constexpr const modelTypeSquare = 1;
    uint8_t constexpr const modelTypeCube = 2;
    std::array<float, 4> constexpr const defaultModelColor{0.5f, 0.5f, 0.5f, 1.0f};

    struct LevelSaveData {
        int m_version;
    };

    struct ModelConfigData {
        std::string modelName;
        uint8_t modelType;
        std::vector<std::string> modelFiles;
        std::vector<std::string> textures;
        std::vector<std::string> alternateTextures;
        std::array<float, 4> defaultColor;
        bool loadFaceNormals;
        bool loadVertexNormals;

        ModelConfigData()
            : modelType{1},
            defaultColor{defaultModelColor}
        {}
    };

    struct LevelConfigData {
        std::vector<ModelConfigData> models;
        bool bounceEnabled;

        // the fraction of the diagonal that the ball should take up.
        float ballSizeDiagonalRatio;

        LevelConfigData()
                : models{},
                  bounceEnabled{false},
                  ballSizeDiagonalRatio{0.0f}
        {
        }

        LevelConfigData(LevelConfigData const &other) noexcept = default;

        LevelConfigData(LevelConfigData &&other) noexcept = default;

        LevelConfigData &operator=(LevelConfigData const &other) noexcept = default;

    };
} // namespace basic

#endif // AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
