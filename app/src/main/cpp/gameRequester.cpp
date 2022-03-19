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

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <string>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "gameRequester.hpp"
#include "drawer.hpp"
#include "renderDetails/renderDetails.hpp"

void handleJNIException(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        throw std::runtime_error("Java exception occurred");
    }
}

template <>
std::string JGameBundle::getDatum<std::string>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jstring> jstr((jstring) lenv->CallObjectMethod(m_bundle.get(), midGetString(),
                                                                    jkey.get()),
                                   deleter);
    handleJNIException(lenv);

    std::string str;
    if (jstr.get() != nullptr) {
        char const *cstr = lenv->GetStringUTFChars(jstr.get(), nullptr);
        handleJNIException(lenv);
        str = cstr;
        lenv->ReleaseStringUTFChars(jstr.get(), cstr);
        handleJNIException(lenv);
    }

    return std::move(str);
}

template <>
std::vector<std::string> JGameBundle::getDatum<std::vector<std::string>>(std::string const &key) const {
    std::vector<std::string> vec;
    std::string str;
    size_t i = 0;
    do {
        str = std::move(getDatum<std::string>(key + std::to_string(i)));
        if (!str.empty()) {
            vec.push_back(str);
        }
        i++;
    } while (!str.empty());

    return std::move(vec);
}

template<>
std::vector<char> JGameBundle::getDatum<std::vector<char>>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jbyteArray> jbytes((jbyteArray)lenv->CallObjectMethod(m_bundle.get(),
                                                                           midGetByteArray(), jkey.get()),
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

    return std::move(bytes);
}

template<>
float JGameBundle::getDatum<float>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    float ret = lenv->CallFloatMethod(m_bundle.get(), midGetFloat(), jkey.get());
    handleJNIException(lenv);
    return ret;
}

template<>
bool JGameBundle::getDatum<bool>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    bool ret = lenv->CallBooleanMethod(m_bundle.get(), midGetBool(), jkey.get());
    handleJNIException(lenv);
    return ret;
}

template<>
int JGameBundle::getDatum<int>(std::string const &key) const {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    int ret = lenv->CallIntMethod(m_bundle.get(), midGetInt(), jkey.get());
    handleJNIException(lenv);
    return ret;
}

template<>
void JGameBundle::putDatum<std::string>(std::string const &key, std::string const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);
    std::shared_ptr<_jstring> jval(lenv->NewStringUTF(val.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), midPutString(), jkey.get(), jval.get());
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<std::vector<std::string>>(std::string const &key, std::vector<std::string> const &val) {
    for (size_t i = 0; i < val.size(); i++) {
        putDatum(key + std::to_string(i), val[i]);
    }
}

template<>
void JGameBundle::putDatum<float>(std::string const &key, float const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), midPutFloat(), jkey.get(), val);
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<std::vector<char>>(std::string const &key, std::vector<char> const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    std::shared_ptr<_jbyteArray> jval(lenv->NewByteArray(val.size()), deleter);
    handleJNIException(lenv);

    jbyte *cval = lenv->GetByteArrayElements(jval.get(), nullptr);
    handleJNIException(lenv);
    memcpy(cval, val.data(), val.size());
    lenv->ReleaseByteArrayElements(jval.get(), cval, JNI_COMMIT);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), midPutByteArray(), jkey.get(), jval.get());
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<bool>(std::string const &key, bool const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), midPutBool(), jkey.get(), val);
    handleJNIException(lenv);
}

template<>
void JGameBundle::putDatum<int>(std::string const &key, int const &val) {
    JNIEnv *lenv = m_requester->env();
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jstring> jkey(lenv->NewStringUTF(key.c_str()), deleter);
    handleJNIException(lenv);

    lenv->CallVoidMethod(m_bundle.get(), midPutInt(), jkey.get(), val);
    handleJNIException(lenv);
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

void JGameRequester::sendKeepAliveEnabled(bool keepAliveEnabled) {
    JNIEnv *lenv = m_env;
    auto deleter = [lenv](jobject obj) {
        lenv->DeleteLocalRef(obj);
    };

    std::shared_ptr<_jclass> notifyClass(m_env->GetObjectClass(m_notify), deleter);
    if (m_env->ExceptionCheck()) {
        m_env->ExceptionClear();
        return;
    }
    jmethodID mid = m_env->GetMethodID(notifyClass.get(), "sendKeepAliveEnabled", "(Z)V");
    if (mid == nullptr) {
        return;
    }
    m_env->CallVoidMethod(m_notify, mid, keepAliveEnabled);
}

void JGameRequester::sendGraphicsDescription(GraphicsDescription const &description,
                                           bool hasAccelerometer, bool isVulkanImplementation) {
    auto bundle = createBundle();
    bundle->putDatum<std::string>(KeyGraphicsName, description.m_graphicsName);
    bundle->putDatum<std::string>(KeyVersionName, description.m_version);
    bundle->putDatum<std::string>(KeyDeviceName, description.m_deviceName);
    bundle->putDatum<std::vector<std::string>>(KeyBugInfo, description.m_extraInfo);
    bundle->putDatum<bool>(KeyHasAccelerometer, hasAccelerometer);
    bundle->putDatum<bool>(KeyIsVulkanImplementation, isVulkanImplementation);
    bundle->putDatum<bool>(KeyIs64Bit, sizeof (void*) == 8);
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
    std::shared_ptr<_jobject> jBundle(m_env->NewObject(bundleClass.get(), mid),
                                        deleter);
    handleJNIException(m_env);

    return std::make_shared<JGameBundle>(shared_from_this(), jBundle, bundleClass);
}

std::shared_ptr<JGameBundle> JGameRequester::createBundle(std::shared_ptr<_jobject> inBundle) {
    return std::make_shared<JGameBundle>(shared_from_this(), std::move(inBundle));
}

JGameBundle::JGameBundle(
    std::shared_ptr<JGameRequester> inRequester,
    std::shared_ptr<_jobject> inBundle,
    std::shared_ptr<_jclass> inBundleClass)
    : m_requester{std::move(inRequester)},
    m_bundle{std::move(inBundle)},
    m_bundleClass{std::move(inBundleClass)},
    m_midGetString{},
    m_midGetFloat{},
    m_midGetByteArray{},
    m_midGetBool{},
    m_midPutString{},
    m_midPutFloat{},
    m_midPutByteArray{},
    m_midPutBool{}
{
    if (!m_bundleClass) {
        JNIEnv *lenv = m_requester->env();
        auto deleter = [lenv](jobject obj) {
            lenv->DeleteLocalRef(obj);
        };
        m_bundleClass = std::shared_ptr<_jclass>(lenv->GetObjectClass(m_bundle.get()), deleter);
        handleJNIException(lenv);
    }
}

jmethodID JGameBundle::mid(jmethodID &m, char const *name, char const *signature) const {
    if (m) {
        return m;
    } else {
        JNIEnv *lenv = m_requester->env();
        m = lenv->GetMethodID(m_bundleClass.get(), name, signature);
        handleJNIException(lenv);
        if (m == nullptr) {
            throw std::runtime_error(std::string("Could not find Bundle::") + name);
        }
        return m;
    }
}

jmethodID JGameBundle::midGetString() const {
    return mid(m_midGetString, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
}

jmethodID JGameBundle::midGetFloat() const {
    return mid(m_midGetFloat, "getFloat", "(Ljava/lang/String;)F");
}

jmethodID JGameBundle::midGetByteArray() const {
    return mid(m_midGetByteArray, "getByteArray", "(Ljava/lang/String;)[B");
}

jmethodID JGameBundle::midGetBool() const {
    return mid(m_midGetBool, "getBoolean", "(Ljava/lang/String;)Z");
}

jmethodID JGameBundle::midGetInt() const {
    return mid(m_midGetInt, "getInt", "(Ljava/lang/String;)I");
}

jmethodID JGameBundle::midPutString() const {
    return mid(m_midPutString, "putString", "(Ljava/lang/String;Ljava/lang/String;)V");
}

jmethodID JGameBundle::midPutFloat() const {
    return mid(m_midPutFloat, "putFloat", "(Ljava/lang/String;F)V");
}

jmethodID JGameBundle::midPutByteArray() const {
    return mid(m_midPutByteArray, "putByteArray", "(Ljava/lang/String;[B)V");
}

jmethodID JGameBundle::midPutBool() const {
    return mid(m_midPutBool, "putBoolean", "(Ljava/lang/String;Z)V");
}

jmethodID JGameBundle::midPutInt() const {
    return mid(m_midPutInt, "putInt", "(Ljava/lang/String;I)V");
}
