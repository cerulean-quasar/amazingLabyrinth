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
#ifndef AMAZING_LABYRINTH_NATIVE_LIB_HPP
#define AMAZING_LABYRINTH_NATIVE_LIB_HPP

#include <cstdint>
#include <string>
#include <map>
#include "common.hpp"
#include "android.hpp"

// These must be the same values as the values in java: MySurfaceCallback.java
std::string const KeyGraphicsName = "graphicsName";
std::string const KeyVersionName = "versionName";
std::string const KeyDeviceName = "deviceName";
std::string const KeyHasAccelerometer = "hasAccelerometer";

class JGameBundle;

class JGameRequester : public std::enable_shared_from_this<JGameRequester>, public GameRequester {
public:
    void sendError(std::string const &error) override;
    void sendError(char const *error) override;
    void sendGraphicsDescription(GraphicsDescription const &description, bool hasAccelerometer) override;
    void sendSaveData(GameBundle const &saveData) override;
    std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
        uint32_t &channels) override;
    std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) override {
        return std::make_unique<AssetStreambuf>(m_assetWrapper->getAsset(file));
    }

    // non-inherited functions.
    boost::optional<GameBundle>  getSaveData();

    // accessors
    JNIEnv *env() { return m_env; }

    // constructors
    JGameRequester(JNIEnv *inEnv, jobject inNotify, std::string inSaveGameFile, AAssetManager *mgr)
            : m_env{inEnv},
              m_notify{inNotify},
              m_pathSaveFile{std::move(inSaveGameFile)},
              m_assetWrapper{new AssetManagerWrapper(mgr)} {}

    ~JGameRequester() override {}
private:
    std::shared_ptr<JGameBundle> createBundle();
    std::shared_ptr<JGameBundle> createBundle(std::shared_ptr<_jobject> inBundle);

    JNIEnv *m_env;
    jobject m_notify;
    std::string m_pathSaveFile;
    std::unique_ptr<AssetManagerWrapper> m_assetWrapper;
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

#endif
