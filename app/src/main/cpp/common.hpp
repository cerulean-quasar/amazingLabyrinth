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
#ifndef AMAZING_LABYRINTH_COMMON_HPP
#define AMAZING_LABYRINTH_COMMON_HPP

#include <string>
#include <memory>
#include <streambuf>
#include "saveData.hpp"

struct GraphicsDescription {
public:
    std::string m_graphicsName;
    std::string m_version;
    std::string m_deviceName;

    GraphicsDescription(std::string inGraphicsName, std::string inVersion, std::string inDeviceName)
        : m_graphicsName{std::move(inGraphicsName)},
          m_version{std::move(inVersion)},
          m_deviceName{std::move(inDeviceName)} {
    }
};

class FileRequester {
public:
    virtual std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) = 0;
    virtual ~FileRequester() = default;
};

class JRequester {
public:
    virtual void sendError(std::string const &error) = 0;
    virtual void sendError(char const *error) = 0;
    virtual void sendGraphicsDescription(GraphicsDescription const &description, bool hasAccelerometer) = 0;
    virtual void sendSaveData(std::vector<uint8_t> const &saveData) = 0;
    virtual std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
            uint32_t &channels) = 0;
    virtual RestoreData getSaveData(Point<uint32_t> const &screenSize) = 0;

    virtual ~JRequester() = default;
};

class GameRequester :public FileRequester, public JRequester {
public:
    ~GameRequester() override = default;
};

#endif // AMAZING_LABYRINTH_COMMON_HPP