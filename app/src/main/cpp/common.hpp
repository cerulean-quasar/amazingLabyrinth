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
#include <map>
#include <boost/variant.hpp>

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

char constexpr const * KeyLevelIdentifier = "LevelIdentifier";
char constexpr const * KeyLevelIsAtStart = "LevelIsStart";
char constexpr const * KeyLevelVersionIdentifier = "LevelVersionIdentifier";
char constexpr const * KeyVersionIdentifier = "VersionIdentifier";

using GameBundleValue = boost::variant<std::string, float, std::vector<char>, bool, int>;
using GameBundleSchema = std::map<std::string, std::type_index>;
using GameBundle = std::map<std::string, GameBundleValue>;

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
    virtual void sendSaveData(GameBundle const &saveData) = 0;
    virtual std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
            uint32_t &channels) = 0;
    virtual ~JRequester() = default;
};

class GameRequester :public FileRequester, public JRequester {
public:
    ~GameRequester() override = default;
};

class GameBundleStringVisitor {
public:
    std::string operator()(std::string str) {
        return std::move(str);
    }

    std::string operator()(float f) {
        return std::to_string(f);
    }

    std::string operator()(std::vector<char> const &data) {
        return std::to_string(data.size());
    }

    std::string operator()(bool b) {
        return std::to_string(b?1:0);
    }

    std::string operator()(int i) {
        return std::to_string(i);
    }
};

class GameBundleFloatVisitor {
public:
    float operator()(std::string const &str) {
        return str.length();
    }

    float operator()(float f) {
        return f;
    }

    float operator()(std::vector<char> const &data) {
        return data.size();
    }

    float operator()(bool b) {
        return b?1.0f:0.0f;
    }

    float operator()(int i) {
        return i;
    }
};

class GameBundleByteArrayVisitor {
public:
    std::vector<char> operator()(std::string const &str) {
        std::vector<char> vec;
        vec.resize(str.length());
        memcpy(vec.data(), str.data(), str.length());
        return std::move(vec);
    }

    std::vector<char> operator()(float f) {
        std::string str = std::to_string(f);
        std::vector<char> vec;
        vec.resize(str.length());
        memcpy(vec.data(), str.data(), str.length());
        return std::move(vec);
    }

    std::vector<char> operator()(std::vector<char> data) {
        return std::move(data);
    }

    std::vector<char> operator()(bool b) {
        std::string str = std::to_string(b?1:0);
        std::vector<char> vec;
        vec.resize(str.length());
        memcpy(vec.data(), str.data(), str.length());
        return std::move(vec);
    }

    std::vector<char> operator()(int i) {
        std::string str = std::to_string(i);
        std::vector<char> vec;
        vec.resize(str.length());
        memcpy(vec.data(), str.data(), str.length());
        return std::move(vec);
    }
};

class GameBundleBoolVisitor {
public:
    bool operator()(std::string const &str) {
        return str.length() > 0;
    }

    bool operator()(float f) {
        return f != 0.0f;
    }

    bool operator()(std::vector<char> const &data) {
        return data.size() > 0;
    }

    bool operator()(bool b) {
        return b;
    }

    bool operator()(int i) {
        return i != 0;
    }
};

class GameBundleIntVisitor {
public:
    int operator()(std::string const &str) {
        return str.length();
    }

    int operator()(float f) {
        return static_cast<int>(std::floor(f));
    }

    int operator()(std::vector<char> const &data) {
        return data.size();
    }

    int operator()(bool b) {
        return b?1:0;
    }

    int operator()(int i) {
        return i;
    }
};

#endif // AMAZING_LABYRINTH_COMMON_HPP