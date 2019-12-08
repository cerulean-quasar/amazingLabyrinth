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
#include "level/levelTracker.hpp"
#include "drawer.hpp"
#include "android.hpp"
#include "gameRequester.hpp"

char constexpr const *DirectorySeparator = "/";
char constexpr const *SaveGameFileName = "amazingLabyrinthSaveGameFile.cbor";

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_startGame(
        JNIEnv *env,
        jobject thisptr,
        jobject jdrawingSurface,
        jobject jassetManager,
        jstring jsaveDataDir,
        jobject jReturnChannel)
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

    GameRequesterCreator requesterCreator([env, jReturnChannel, saveDataFile, manager]
        (Graphics *inGraphics) -> std::shared_ptr<GameRequester> {
        return std::make_shared<JGameRequester>(env, jReturnChannel, saveDataFile, manager,
                inGraphics);
    });

    ANativeWindow *window = ANativeWindow_fromSurface(env, jdrawingSurface);
    if (window == nullptr) {
        JGameRequester requester(env, jReturnChannel, saveDataFile, manager, nullptr);
        requester.sendError("Unable to create window from surface.");
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
        GameWorker worker{surface, requesterCreator, true, /* useLegacy */ false};
        worker.drawingLoop();
    } catch (std::runtime_error &e) {
        // TODO: manage exception handling in Graphics.
        JGameRequester requester(env, jReturnChannel, saveDataFile, manager, nullptr);
        requester.sendError(e.what());
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerStop(
        JNIEnv *env,
        jclass jclassptr)
{
    gameFromGuiChannel().sendStopDrawingEvent();
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSendSaveData(
        JNIEnv *env,
        jclass jclassptr)
{
    auto ev = std::make_shared<SaveLevelDataEvent>();
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSurfaceChanged(
        JNIEnv *env,
        jclass jclassptr,
        jint jwidth,
        jint jheight)
{
    auto ev = std::make_shared<SurfaceChangedEvent>(jwidth, jheight);
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_tellDrawerSwitchLevel(
        JNIEnv *env,
        jclass jclassptr,
        jint jlevel)
{
    auto ev = std::make_shared<LevelChangedEvent>(jlevel);
    gameFromGuiChannel().sendEvent(ev);
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_getLevelList(
        JNIEnv *env,
        jclass jclassptr)
{
    std::vector<std::string> levels = LevelTracker::getLevelDescriptions();
    jobjectArray ret = (jobjectArray)env->NewObjectArray(levels.size(), env->FindClass("java/lang/String"), nullptr);
    for (int i = 0; i < levels.size(); i++) {
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(levels[i].c_str()));
    }

    return ret;
}

