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

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "levelTracker/levelTracker.hpp"
#include "drawer.hpp"
#include "android.hpp"
#include "gameRequester.hpp"

char constexpr const *DirectorySeparator = "/";
char constexpr const *SaveGameFileName = "amazingLabyrinthSaveGameFile.cbor";

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_startGame(
        JNIEnv *env,
        jobject,
        jobject jdrawingSurface,
        jobject jassetManager,
        jstring jsaveDataDir,
        jobject jReturnChannel,
        jfloat jrotationAngle)
{
    std::string saveDataFile;
    try {
        char const *csaveDataDir = env->GetStringUTFChars(jsaveDataDir, nullptr);
        handleJNIException(env);
        saveDataFile += csaveDataDir;
        env->ReleaseStringUTFChars(jsaveDataDir, csaveDataDir);
        saveDataFile += DirectorySeparator;
        saveDataFile += SaveGameFileName;
    } catch (std::runtime_error &e) {
        // no way to send the error yet...
        return;
    }

    // Stays active for the duration of this function, does not have a deleter.
    // At the end of this function, the source java object and its C object
    // become invalid.  This function only exits when the game exits.
    AAssetManager *manager = AAssetManager_fromJava(env, jassetManager);
    if (manager == nullptr) {
        return;
    }

    auto gameRequester = std::make_shared<JGameRequester>(env, jReturnChannel, saveDataFile, manager);

    ANativeWindow *window = ANativeWindow_fromSurface(env, jdrawingSurface);
    if (window == nullptr) {
        gameRequester->sendError("Unable to create window from surface.");
        return;
    }
    auto deleter = [](ANativeWindow *windowRaw) {
        /* release the java window object */
        if (windowRaw != nullptr) {
            ANativeWindow_release(windowRaw);
        }
    };
    std::shared_ptr<WindowType> surface(window, deleter);

    try {
        GameWorker worker{surface, gameRequester, true, /* useLegacy */ false, jrotationAngle};
        worker.drawingLoop();
    } catch (std::runtime_error &e) {
        gameRequester->sendError(e.what());
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerStop(
        JNIEnv *,
        jclass)
{
    gameFromGuiChannel().sendStopDrawingEvent();
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSendSaveData(
        JNIEnv *,
        jclass)
{
    auto ev = std::make_shared<SaveLevelDataEvent>();
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSurfaceChanged(
        JNIEnv *,
        jclass,
        jint jwidth,
        jint jheight,
        jfloat jrotationAngle)
{
    auto ev = std::make_shared<SurfaceChangedEvent>(jwidth, jheight, jrotationAngle);
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSwitchLevel(
        JNIEnv *env,
        jclass,
        jstring jlevel)
{
    char const *clevel = env->GetStringUTFChars(jlevel, nullptr);
    handleJNIException(env);
    std::string level(clevel);
    env->ReleaseStringUTFChars(jlevel, clevel);

    auto ev = std::make_shared<LevelChangedEvent>(std::move(level));
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerDragOccurred(
        JNIEnv *,
        jclass,
        jfloat jStartX,
        jfloat jStartY,
        jfloat jDistanceX,
        jfloat jDistanceY)
{
    auto ev = std::make_shared<DragEvent>(jStartX, jStartY, jDistanceX, jDistanceY);
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerDragEnded(
        JNIEnv *,
        jclass,
        jfloat jPositionX,
        jfloat jPositionY)
{
    auto ev = std::make_shared<DragEndedEvent>(jPositionX, jPositionY);
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerTapOccurred(
        JNIEnv *,
        jclass,
        jfloat jPositionX,
        jfloat jPositionY)
{
    auto ev = std::make_shared<TapEvent>(jPositionX, jPositionY);
    gameFromGuiChannel().sendEvent(ev);
}
