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

#include <jni.h>
#include <native_window.h>
#include <native_window_jni.h>

#include <string>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "gameRequester.hpp"
#include "drawer.hpp"
#include "level/levelTracker.hpp"
#include "saveData.hpp"
#include "serializeSaveData.hpp"

void handleJNIException(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        throw std::runtime_error("Java exception occurred");
    }
}

template <>
GameBundleValue JGameBundle::getDatum<std::string>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jstring> jstr((jstring) lenv->CallObjectMethod(m_bundle.get(), m_midGetString,
                                                                    jkey.get()),
                                   deleter);
    handleJNIException(lenv);

    char const *cstr = lenv->GetStringUTFChars(jstr.get(), nullptr);
    handleJNIException(lenv);
    std::string str{cstr};
    lenv->ReleaseStringUTFChars(jstr.get(), cstr);
    handleJNIException(lenv);

    return GameBundleValue{std::move(str)};
}

template<>
GameBundleValue JGameBundle::getDatum<std::vector<char>>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jbyteArray> jbytes((jbyteArray)lenv->CallObjectMethod(m_bundle.get(),
                                                                           m_midGetByteArray, jkey.get()),
                                        deleter);
    handleJNIException(lenv);

    jint size = lenv->GetArrayLength(jbytes.get());
    handleJNIException(lenv);

    std::vector<char> bytes;
    bytes.resize(static_cast<size_t >(size));
    jbyte *cbytes = lenv->GetByteArrayElements(jbytes.get(), nullptr);
    handleJNIException(lenv);
    memcpy(bytes.data(), cbytes, static_cast<size_t>(size));
    lenv->ReleaseByteArrayElements(jbytes.get(), cbytes, JNI_ABORT);
    handleJNIException(lenv);

    return GameBundleValue{std::move(bytes)};
}

template<>
GameBundleValue JGameBundle::getDatum<float>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    float ret = lenv->CallFloatMethod(m_bundle.get(), m_midGetFloat, jkey.get());
    handleJNIException(lenv);
    return GameBundleValue{ret};
}

template<>
GameBundleValue JGameBundle::getDatum<bool>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    bool ret = lenv->CallBooleanMethod(m_bundle.get(), m_midGetBool, jkey.get());
    handleJNIException(lenv);
    return GameBundleValue{ret};
}

template<>
GameBundleValue JGameBundle::getDatum<int>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    int ret = lenv->CallIntMethod(m_bundle.get(), m_midGetInt, jkey.get());
    handleJNIException(lenv);
    return GameBundleValue{ret};
}

template<>
void JGameBundle::putDatum<std::string>(std::string const &key, GameBundleValue const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::string str{boost::apply_visitor(GameBundleStringVisitor{}, val)};
    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jstring> jval(lenv->NewStringUTF(str.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), m_midPutString, jkey.get(), jval.get());
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<float>(std::string const &key, GameBundleValue const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    float f = boost::apply_visitor(GameBundleFloatVisitor{}, val);

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), m_midPutFloat, jkey.get(), f);
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<std::vector<char>>(std::string const &key, GameBundleValue const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::vector<char> vec{boost::get<std::vector<char>>(val)};
    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    std::shared_ptr<_jbyteArray> jval(lenv->NewByteArray(vec.size()), deleter);
    handleJNIException(lenv);

    jbyte *cval = lenv->GetByteArrayElements(jval.get(), nullptr);
    handleJNIException(lenv);
    memcpy(cval, vec.data(), vec.size());
    lenv->ReleaseByteArrayElements(jval.get(), cval, JNI_COMMIT);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), m_midPutByteArray, jkey.get(), jval.get());
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<bool>(std::string const &key, GameBundleValue const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    bool f = boost::apply_visitor(GameBundleBoolVisitor{}, val);

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), m_midPutBool, jkey.get(), f);
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<int>(std::string const &key, GameBundleValue const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    int i = boost::apply_visitor(GameBundleIntVisitor{}, val);

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), m_midPutInt, jkey.get(), i);
    handleJNIException(lenv);
}

std::shared_ptr<TextureData> JGameRequester::getDepthTexture(
        DrawObjectTable const &objsData,
        float width,
        float height)
{
    m_graphics->getDepthTexture(objsData, width, height);
}

std::vector<char> JGameRequester::getTextImage(std::string text, uint32_t &width, uint32_t &height, uint32_t &channels) {
    JNIEnv *lenv = m_env;
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };
    std::shared_ptr<_jclass> imageLoaderClass(
            m_env->FindClass("com/quasar/cerulean/amazinglabyrinth/TextImageLoader"), deleter);
    handleJNIException(m_env);
    jmethodID mid = m_env->GetMethodID(imageLoaderClass.get(), "<init>", "(Ljava/lang/String;)V");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    std::shared_ptr<_jstring> jtext(m_env->NewStringUTF(text.c_str()), deleter);
    handleJNIException(m_env);
    std::shared_ptr<_jobject> imageLoader(
            m_env->NewObject(imageLoaderClass.get(), mid, jtext.get()),
            deleter);
    handleJNIException(m_env);

    mid = m_env->GetMethodID(imageLoaderClass.get(), "getImageSize", "()I");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jsize = m_env->CallIntMethod(imageLoader.get(), mid);
    handleJNIException(m_env);

    mid = m_env->GetMethodID(imageLoaderClass.get(), "getImageWidth", "()I");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jwidth = m_env->CallIntMethod(imageLoader.get(), mid);
    handleJNIException(m_env);
    width = static_cast<uint32_t> (jwidth);

    mid = m_env->GetMethodID(imageLoaderClass.get(), "getImageHeight", "()I");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jheight = m_env->CallIntMethod(imageLoader.get(), mid);
    handleJNIException(m_env);
    height = static_cast<uint32_t> (jheight);

    std::shared_ptr<_jbyteArray> jimageData(m_env->NewByteArray(jsize), deleter);
    handleJNIException(m_env);
    mid = m_env->GetMethodID(imageLoaderClass.get(), "getImageData", "([B)V");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    m_env->CallVoidMethod(imageLoader.get(), mid, jimageData.get());
    handleJNIException(m_env);

    std::vector<char> imageData;
    auto size = static_cast<size_t> (jsize);
    imageData.resize(size);
    jbyte *bytes = m_env->GetByteArrayElements(jimageData.get(), nullptr);
    handleJNIException(m_env);
    memcpy(imageData.data(), bytes, size);
    m_env->ReleaseByteArrayElements(jimageData.get(), bytes, JNI_ABORT);
    handleJNIException(m_env);

    channels = 4;
    return imageData;
}

void JGameRequester::sendError(std::string const &error) {
    sendError(error.c_str());
}

void JGameRequester::sendError(char const *error) {
    // don't throw in here because we might be handling an exception already.
    JNIEnv *lenv = m_env;
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jerror(m_env->NewStringUTF(error), deleter);
    if (m_env->ExceptionCheck()) {
        m_env->ExceptionClear();
        return;
    }
    std::shared_ptr<_jclass> notifyClass(m_env->GetObjectClass(m_notify), deleter);
    if (m_env->ExceptionCheck()) {
        m_env->ExceptionClear();
        return;
    }
    jmethodID mid = m_env->GetMethodID(notifyClass.get(), "sendError", "(Ljava/lang/String;)V");
    if (mid == nullptr) {
        return;
    }
    m_env->CallVoidMethod(m_notify, mid, jerror.get());
    if (m_env->ExceptionCheck()) {
        m_env->ExceptionClear();
        return;
    }
}

void JGameRequester::sendGraphicsDescription(GraphicsDescription const &description,
                                           bool hasAccelerometer) {
    auto bundle = createBundle();
    bundle->putDatum<std::string>(KeyGraphicsName, GameBundleValue{description.m_graphicsName});
    bundle->putDatum<std::string>(KeyVersionName, GameBundleValue{description.m_version});
    bundle->putDatum<std::string>(KeyDeviceName, GameBundleValue{description.m_deviceName});
    bundle->putDatum<bool>(KeyHasAccelerometer, GameBundleValue{hasAccelerometer});
    JNIEnv *lenv = m_env;
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jclass> notifyClass(m_env->GetObjectClass(m_notify), deleter);
    handleJNIException(m_env);
    jmethodID mid = m_env->GetMethodID(notifyClass.get(), "sendBundle", "(Landroid/os/Bundle;)V");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find API to send graphics description."));
    }
    m_env->CallVoidMethod(m_notify, mid, bundle->bundle().get());
    handleJNIException(m_env);
}

std::shared_ptr<JGameBundle> JGameRequester::createBundle() {
    JNIEnv *lenv = m_env;
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };
    std::shared_ptr<_jclass> bundleClass(m_env->FindClass("android/os/Bundle"), deleter);
    handleJNIException(m_env);

    jmethodID mid = m_env->GetMethodID(bundleClass.get(), "<init>", "()V");
    if (mid == nullptr) {
        throw (std::runtime_error("Could not find method for creating a Bundle."));
    }
    std::shared_ptr<_jobject> jSaveData(m_env->NewObject(bundleClass.get(), mid),
                                        deleter);
    handleJNIException(m_env);

    return std::make_shared<JGameBundle>(shared_from_this(), jSaveData);
}

std::shared_ptr<JGameBundle> JGameRequester::createBundle(std::shared_ptr<_jobject> inBundle) {
    return std::make_shared<JGameBundle>(shared_from_this(), std::move(inBundle));
}

void JGameRequester::sendSaveData(std::vector<uint8_t> const &saveData) {
    std::ofstream saveDataStream(m_pathSaveFile);

    if (!saveDataStream.fail()) {
        saveDataStream.write(reinterpret_cast<char const *>(saveData.data()), saveData.size());
    }
}

RestoreData JGameRequester::getSaveData(Point<uint32_t> const &screenSize) {
    std::ifstream saveDataStream(m_pathSaveFile, std::ifstream::binary);

    if (saveDataStream.good()) {
        saveDataStream.seekg(0, saveDataStream.end);
        size_t i = static_cast<size_t >(saveDataStream.tellg());
        saveDataStream.seekg(0, saveDataStream.beg);
        std::vector<uint8_t> vec;
        vec.resize(i);
        saveDataStream.read(reinterpret_cast<char *>(vec.data()), vec.size());

        if (!saveDataStream.fail()) {
            return createLevelFromRestore(vec, screenSize);
        }
    }

    return createLevelFromRestore();
}

JGameBundle::JGameBundle(std::shared_ptr<JGameRequester> inRequester, std::shared_ptr<_jobject> inBundle)
    : m_requester{std::move(inRequester)},
    m_bundle{std::move(inBundle)},
    m_bundleClass{},
    m_midGetString{},
    m_midGetFloat{},
    m_midGetByteArray{},
    m_midGetBool{},
    m_midPutString{},
    m_midPutFloat{},
    m_midPutByteArray{},
    m_midPutBool{}
{
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };
    m_bundleClass = std::shared_ptr<_jclass>(lenv->GetObjectClass(m_bundle.get()), deleter);
    handleJNIException(lenv);

    m_midGetString = m_requester->env()->GetMethodID(m_bundleClass.get(), "getString",
                                                     "(Ljava/lang/String;)Ljava/lang/String;");
    m_midGetFloat = m_requester->env()->GetMethodID(m_bundleClass.get(), "getFloat",
                                                    "(Ljava/lang/String;)F");
    m_midGetByteArray = m_requester->env()->GetMethodID(m_bundleClass.get(), "getByteArray",
                                                        "(Ljava/lang/String;)[B");
    m_midGetBool = m_requester->env()->GetMethodID(m_bundleClass.get(), "getBoolean",
                                                        "(Ljava/lang/String;)Z");
    m_midGetInt = m_requester->env()->GetMethodID(m_bundleClass.get(), "getInt",
                                                  "(Ljava/lang/String;)I");
    m_midPutString = m_requester->env()->GetMethodID(m_bundleClass.get(), "putString",
                                                     "(Ljava/lang/String;Ljava/lang/String;)V");
    m_midPutFloat = m_requester->env()->GetMethodID(m_bundleClass.get(), "putFloat",
                                                    "(Ljava/lang/String;F)V");
    m_midPutByteArray = m_requester->env()->GetMethodID(m_bundleClass.get(), "putByteArray",
                                                        "(Ljava/lang/String;[B)V");
    m_midPutBool = m_requester->env()->GetMethodID(m_bundleClass.get(), "putBoolean",
                                                   "(Ljava/lang/String;Z)V");
    m_midPutInt = m_requester->env()->GetMethodID(m_bundleClass.get(), "putInt",
                                                   "(Ljava/lang/String;I)V");

    if (m_midGetString == nullptr || m_midGetFloat == nullptr || m_midGetByteArray == nullptr ||
        m_midGetBool == nullptr || m_midGetInt == nullptr ||
        m_midPutString == nullptr || m_midPutFloat == nullptr || m_midPutByteArray == nullptr ||
        m_midPutBool == nullptr || m_midGetInt == nullptr) {
        throw std::runtime_error("Could not initialize save data handle");
    }

    insertGetDatumMapEntry<std::string>(m_getDatumMap);
    insertGetDatumMapEntry<float>(m_getDatumMap);
    insertGetDatumMapEntry<std::vector<char>>(m_getDatumMap);
    insertGetDatumMapEntry<bool>(m_getDatumMap);
    insertGetDatumMapEntry<int>(m_getDatumMap);

    insertPutDatumMapEntry<std::string>(m_putDatumMap);
    insertPutDatumMapEntry<float>(m_putDatumMap);
    insertPutDatumMapEntry<std::vector<char>>(m_putDatumMap);
    insertPutDatumMapEntry<bool>(m_putDatumMap);
    insertPutDatumMapEntry<int>(m_putDatumMap);
}

GameBundle JGameBundle::getData(GameBundleSchema const &keys) const {
    GameBundle ret;
    for (auto const &key : keys) {
        auto fcnEntry = m_getDatumMap.find(key.second);
        if (fcnEntry == m_getDatumMap.end()) {
            throw std::runtime_error("Could not find function to obtain data.");
        }
        GameBundleValue val = (this->*(fcnEntry->second))(key.first);
        ret.insert(std::make_pair(key.first, val));
    }

    return ret;
}

void JGameBundle::putData(GameBundle const &data) {
    for (auto const &datum : data) {
        auto fcnEntry = m_putDatumMap.find(std::type_index(datum.second.type()));
        if (fcnEntry == m_putDatumMap.end()) {
            throw std::runtime_error("Could not find function to put data.");
        }

        (this->*(fcnEntry->second))(datum.first, datum.second);
    }
}
