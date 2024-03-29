/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

// These must be the same values as the values in java: MySurfaceCallback.java
std::string const KeyGraphicsName = "graphicsName";
std::string const KeyVersionName = "versionName";
std::string const KeyDeviceName = "deviceName";
std::string const KeyBugInfo = "bugInfo";
std::string const KeyHasAccelerometer = "hasAccelerometer";
std::string const KeyIsVulkanImplementation = "isVulkan";
std::string const KeyIs64Bit = "is64Bit";

void handleJNIException(JNIEnv *env);

class JGameBundle;
class JStringArrayList;

class JGameRequester : public std::enable_shared_from_this<JGameRequester>, public GameRequester {
public:
    void sendError(std::string const &error) override;
    void sendError(char const *error) override;
    void sendGraphicsDescription(GraphicsDescription const &description, bool hasAccelerometer, bool isVulkanImplementation) override;
    void sendKeepAliveEnabled(bool keepAliveEnabled) override;
    std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height,
        uint32_t &channels) override;
    std::unique_ptr<std::streambuf> getAssetStream(std::string const &file) override {
        return std::make_unique<AssetStreambuf>(m_assetWrapper->getAsset(file));
    }
    std::unique_ptr<std::streambuf> getLevelTableAssetStream() override {
        return getAssetStream(m_levelTableFilePath);
    }
    std::string getSaveDataFileName() override { return m_pathSaveFile; }

    // accessors
    JNIEnv *env() { return m_env; }

    // constructors
    JGameRequester(JNIEnv *inEnv, jobject inNotify, std::string inSaveGameFile, AAssetManager *mgr)
            : m_env{inEnv},
              m_notify{inNotify},
              m_pathSaveFile{std::move(inSaveGameFile)},
              m_levelTableFilePath{"configs/levels.json"},
              m_assetWrapper{new AssetManagerWrapper(mgr)} {}

    ~JGameRequester() override {}
private:
    std::shared_ptr<JGameBundle> createBundle();
    std::shared_ptr<JGameBundle> createBundle(std::shared_ptr<_jobject> inBundle);

    JNIEnv *m_env;
    jobject m_notify;
    std::string m_pathSaveFile;
    std::string m_levelTableFilePath;
    std::unique_ptr<AssetManagerWrapper> m_assetWrapper;
};

class JGameBundle {
public:
    template <typename T>
    T getDatum(std::string const &key) const;

    template <typename T>
    void putDatum(std::string const &, T const &) {
        throw std::runtime_error("JGameBundle: unimplemented type for putDatum");
    }

    // accessors
    std::shared_ptr<_jobject> const &bundle() { return m_bundle; }

    JGameBundle(std::shared_ptr<JGameRequester> inRequester, std::shared_ptr<_jobject> inBundle,
            std::shared_ptr<_jclass> inBundleClass = nullptr);
private:
    std::shared_ptr<JGameRequester> m_requester;
    std::shared_ptr<_jobject> m_bundle;
    std::shared_ptr<_jclass> m_bundleClass;
    mutable jmethodID m_midGetString;
    mutable jmethodID m_midGetFloat;
    mutable jmethodID m_midGetByteArray;
    mutable jmethodID m_midGetBool;
    mutable jmethodID m_midGetInt;

    mutable jmethodID m_midPutString;
    mutable jmethodID m_midPutFloat;
    mutable jmethodID m_midPutByteArray;
    mutable jmethodID m_midPutBool;
    mutable jmethodID m_midPutInt;

    jmethodID mid(jmethodID &m, char const *name, char const *signature) const;
    jmethodID midGetString() const;
    jmethodID midGetFloat() const;
    jmethodID midGetByteArray() const;
    jmethodID midGetBool() const;
    jmethodID midGetInt() const;

    jmethodID midPutString() const;
    jmethodID midPutFloat() const;
    jmethodID midPutByteArray() const;
    jmethodID midPutBool() const;
    jmethodID midPutInt() const;
};

#endif // AMAZING_LABYRINTH_GAME_REQUESTER_HPP
