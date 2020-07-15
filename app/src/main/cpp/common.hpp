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
#ifndef AMAZING_LABYRINTH_COMMON_HPP
#define AMAZING_LABYRINTH_COMMON_HPP

#include <string>
#include <memory>
#include <streambuf>

class Graphics;

char constexpr const *shadowsChainingRenderDetailsName = "shadowsChaining";
char constexpr const *shadowsRenderDetailsName = "shadows";
char constexpr const *objectWithShadowsRenderDetailsName = "objectWithShadows";

struct GraphicsDescription {
public:
    std::string m_graphicsName;
    std::string m_version;
    std::string m_deviceName;
    std::vector<std::string> m_extraInfo;

    GraphicsDescription(std::string inGraphicsName, std::string inVersion, std::string inDeviceName,
            std::vector<std::string> &&inExtraInfo)
        : m_graphicsName{std::move(inGraphicsName)},
          m_version{std::move(inVersion)},
          m_deviceName{std::move(inDeviceName)},
          m_extraInfo{std::move(inExtraInfo)}
    {
    }
};

class GraphicsRequester {
public:
    virtual void getDepthTexture(
            DrawObjectTable const &objsData,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            float farthestDepth,
            float nearestDepth,
            std::vector<float> &depthMap,
            std::vector<glm::vec3> &normalMap) = 0;
    virtual ~GraphicsRequester() = default;
};

class FileRequester {
public:
    virtual std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) = 0;
    virtual std::unique_ptr<std::streambuf> getLevelTableAssetStream() = 0;
    virtual std::string getSaveDataFileName() = 0;
    virtual ~FileRequester() = default;
};

class JRequester {
public:
    virtual void sendError(std::string const &error) = 0;
    virtual void sendError(char const *error) = 0;
    virtual void sendGraphicsDescription(GraphicsDescription const &description, bool hasAccelerometer) = 0;
    virtual void sendKeepAliveEnabled(bool keepAliveEnabled) = 0;
    virtual std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
            uint32_t &channels) = 0;

    virtual ~JRequester() = default;
};

class GameRequester :public FileRequester, public JRequester, public GraphicsRequester {
public:
    ~GameRequester() override = default;
};

using GameRequesterCreator = std::function<std::shared_ptr<GameRequester>(Graphics *)>;

std::vector<char> readFile(std::shared_ptr<FileRequester> const &requester, std::string const &filename);
void checkGraphicsError();
#endif // AMAZING_LABYRINTH_COMMON_HPP