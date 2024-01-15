/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#define AMAZING_LABYRINTH_LEVEL_TRACKER_HPP
#include <array>
#include <fstream>
#include <functional>
#include <boost/optional.hpp>
#include "../levels/finisher/types.hpp"
#include "../levels/basic/level.hpp"
#include "../common.hpp"
#include "../mathGraphics.hpp"

namespace levelTracker {
    using GenerateLevelFcn = std::function<std::shared_ptr<basic::Level>(levelDrawer::Adaptor)>;
    using GenerateFinisherFcn = std::function<std::shared_ptr<finisher::LevelFinisher>(
            levelDrawer::Adaptor, float, float, float)>;

    struct LevelGroup {
        GenerateLevelFcn getStarterFcn;
        GenerateLevelFcn getLevelFcn;
        GenerateFinisherFcn getFinisherFcn;
    };

    struct LevelTableEntry {
        std::string levelName;
        std::string fileName;

        LevelTableEntry() = default;
        LevelTableEntry(LevelTableEntry &&other) noexcept
                : levelName{std::move(other.levelName)},
                  fileName{std::move(other.fileName)}
        {}

        LevelTableEntry &operator=(LevelTableEntry &&other) noexcept {
            levelName = std::move(other.levelName);
            fileName = std::move(other.fileName);

            return *this;
        }
    };

    void saveGameData(
            std::shared_ptr<FileRequester> const &requester,
            uint32_t screenWidth,
            uint32_t screenHeight,
            std::string const &levelName,
            std::shared_ptr<basic::Level> const &level,
            bool needsStarter);

    class Loader {
    public:
        void gotoNextLevel();

        bool setLevel(std::string levelName) {
            auto levelNumber = getLevelNumber(levelName);
            if (levelNumber != boost::none) {
                m_currentLevel = levelNumber;
                return true;
            }
            return false;
        }

        std::string levelName() {
            if (m_currentLevel != boost::none) {
                return m_levelTable[m_currentLevel.get()].levelName;
            } else if (!m_levelTable.empty()){
                return m_levelTable[0].levelName;
            } else {
                return "";
            }
        }

        bool validLevel(std::string const &level) {
            auto levelNumber = getLevelNumber(level);
            return levelNumber != boost::none;
        }

        boost::optional<size_t> getLevelNumber(std::string const &levelName) {
            size_t i = 0;
            for (auto const &entry : m_levelTable) {
                if (levelName == entry.levelName) {
                    return i;
                }
                i++;
            }
            return boost::none;
        }

        LevelGroup getLevelGroupFcns(uint32_t screenWidth, uint32_t screenHeight);

        Loader(std::shared_ptr<GameRequester> inGameRequester, bool shadowsEnabled);

    private:
        std::shared_ptr<GameRequester> m_gameRequester;
        std::vector<LevelTableEntry> m_levelTable;
        boost::optional<size_t> m_currentLevel;
        bool m_shadowsEnabled;

        std::vector<uint8_t> getDataFromFile(std::string const &filename) {
            std::ifstream stream(filename, std::ifstream::binary);
            auto data = getDataFromFile(stream);
            stream.close();
            return data;
        }

        std::vector<uint8_t> getDataFromFile(std::unique_ptr<std::streambuf> const &sb) {
            std::istream stream(sb.get());
            return getDataFromFile(stream);
        }

        std::vector<uint8_t> getDataFromFile(std::istream &stream) {
            if (stream.good()) {
                stream.seekg(0, stream.end);
                size_t i = static_cast<size_t >(stream.tellg());
                stream.seekg(0, stream.beg);
                std::vector<uint8_t> vec;
                vec.resize(i);
                stream.read(reinterpret_cast<char *>(vec.data()), vec.size());

                if (!stream.fail()) {
                    return vec;
                }
            }

            return std::vector<uint8_t>{};
        }

    };
}
#endif
