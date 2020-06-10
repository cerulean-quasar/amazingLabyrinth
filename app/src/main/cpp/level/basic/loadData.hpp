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

#ifndef AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
#define AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
namespace basic {
    struct LevelSaveData {
        int m_version;
    };

    struct LevelConfigData {
        std::string m_ballModel;
        std::string m_ballTexture;
        bool m_bounceEnabled;

        // the fraction of the diagonal that the ball should take up.
        float m_ballSizeDiagonalRatio;

        LevelConfigData()
                : m_ballTexture{},
                  m_ballModel{},
                  m_bounceEnabled{false},
                  m_ballSizeDiagonalRatio{0.0f}
        {
        }

        LevelConfigData(LevelConfigData &&other) noexcept
                : m_ballTexture{std::move(other.m_ballTexture)},
                  m_ballModel{std::move(other.m_ballTexture)},
                  m_bounceEnabled{other.m_bounceEnabled},
                  m_ballSizeDiagonalRatio{other.m_ballSizeDiagonalRatio}
        {
        }

        LevelConfigData(
                std::string ballModel,
                std::string ballTexture,
                bool bounceEnabled,
                float ballSizeDiagonalRatio) noexcept
                : m_ballModel{std::move(ballTexture)},
                  m_ballTexture{std::move(ballTexture)},
                  m_bounceEnabled{bounceEnabled},
                  m_ballSizeDiagonalRatio{ballSizeDiagonalRatio}
        {
        }
    };
} // namespace basic

#endif // AMAZING_LABYRINTH_BASIC_LOAD_DATA_HPP
