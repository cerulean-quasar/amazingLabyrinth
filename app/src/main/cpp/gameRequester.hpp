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
#ifndef AMAZING_LABYRINTH_GAME_REQUESTER_HPP
#define AMAZING_LABYRINTH_GAME_REQUESTER_HPP

#include <cstdint>
#include <string>
#include <map>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include "common.hpp"
#include "android.hpp"
#include "saveData.hpp"

// These must be the same values as the values in java: MySurfaceCallback.java
std::string const KeyGraphicsName = "graphicsName";
std::string const KeyVersionName = "versionName";
std::string const KeyDeviceName = "deviceName";
std::string const KeyHasAccelerometer = "hasAccelerometer";

void handleJNIException(JNIEnv *env);

class JGameBundle;

class JGameRequester : public std::enable_shared_from_this<JGameRequester>, public GameRequester {
public:
    void sendError(std::string const &error) override;
    void sendError(char const *error) override;
    void sendGraphicsDescription(GraphicsDescription const &description, bool hasAccelerometer) override;
    void sendSaveData(std::vector<uint8_t> const &saveData) override;
    std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
        uint32_t &channels) override;
    std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) override {
        return std::make_unique<AssetStreambuf>(m_assetWrapper->getAsset(file));
    }
    RestoreData getSaveData(Point<uint32_t> const &screenSize) override;
    std::shared_ptr<TextureData> getDepthTexture(
            DrawObjectTable const &objsData,
            float width,
            float height,
            uint32_t nbrSamplesForWidth,
            std::vector<float> &depthMap,
            std::vector<glm::vec3> &normalMap) override;

    // accessors
    JNIEnv *env() { return m_env; }

    // constructors
    JGameRequester(JNIEnv *inEnv, jobject inNotify, std::string inSaveGameFile, AAssetManager *mgr,
            Graphics *inGraphics)
            : m_env{inEnv},
              m_notify{inNotify},
              m_pathSaveFile{std::move(inSaveGameFile)},
              m_assetWrapper{new AssetManagerWrapper(mgr)},
              m_graphics(inGraphics) {}

    ~JGameRequester() override {}
private:
    std::shared_ptr<JGameBundle> createBundle();
    std::shared_ptr<JGameBundle> createBundle(std::shared_ptr<_jobject> inBundle);

    JNIEnv *m_env;
    jobject m_notify;
    std::string m_pathSaveFile;
    std::unique_ptr<AssetManagerWrapper> m_assetWrapper;
    Graphics *m_graphics;
};

using GameBundleValue = boost::variant<std::string, float, std::vector<char>, bool, int>;
using GameBundleSchema = std::map<std::string, std::type_index>;
using GameBundle = std::map<std::string, GameBundleValue>;

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

class JGameBundle {
public:
    GameBundle getData(GameBundleSchema const &keys) const;

    template <typename T>
    GameBundleValue getDatum(std::string const &key) const;

    void putData(GameBundle const &val);

    template <typename T>
    void putDatum(std::string const &key, GameBundleValue const &val);

    // accessors
    std::shared_ptr<_jobject> const &bundle() { return m_bundle; }

    JGameBundle(std::shared_ptr<JGameRequester> inRequester, std::shared_ptr<_jobject> inBundle);
    using GetMap = std::map<std::type_index, GameBundleValue (JGameBundle::*)(std::string const &key) const>;
    using PutMap = std::map<std::type_index, void (JGameBundle::*)(std::string const &key, GameBundleValue const &val)>;
private:
    std::map<std::type_index, GameBundleValue (JGameBundle::*)(std::string const &key) const> m_getDatumMap;
    std::map<std::type_index, void (JGameBundle::*)(std::string const &key, GameBundleValue const &val)> m_putDatumMap;
    std::shared_ptr<JGameRequester> m_requester;
    std::shared_ptr<_jobject> m_bundle;
    std::shared_ptr<_jclass> m_bundleClass;
    jmethodID m_midGetString;
    jmethodID m_midGetFloat;
    jmethodID m_midGetByteArray;
    jmethodID m_midGetBool;
    jmethodID m_midGetInt;

    jmethodID m_midPutString;
    jmethodID m_midPutFloat;
    jmethodID m_midPutByteArray;
    jmethodID m_midPutBool;
    jmethodID m_midPutInt;

};

template <typename T> void insertGetDatumMapEntry(JGameBundle::GetMap &map) {
    map.insert(std::make_pair(std::type_index(typeid(T)), &JGameBundle::getDatum<T>));
}

template <typename T> void insertPutDatumMapEntry(JGameBundle::PutMap &map) {
    map.insert(std::make_pair(std::type_index(typeid(T)), &JGameBundle::putDatum<T>));
}

#endif // AMAZING_LABYRINTH_GAME_REQUESTER_HPP
