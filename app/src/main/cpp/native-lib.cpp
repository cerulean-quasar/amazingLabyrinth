/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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
#include <atomic>
#include <memory>
#include <looper.h>
#include <sensor.h>


#include "graphics.hpp"
#include "graphicsGL.hpp"
#include "graphicsVulkan.hpp"

/* Used to communicate between the gui thread and the drawing thread.  When the GUI thread wants
 * the drawer to stop drawing, cleanup, and exit, it sets this value to true.  The GUI will set
 * this value to false before starting a new drawer.
 */
std::atomic<bool> stopDrawing(false);

/* The following data are per program data and are to be accessed by one thread at a time.
 * Note: there are only two threads in this program; the drawer and the GUI.  Some of the data
 * below are set up by one thread and accessed by another but are guaranteed to not be accessed at
 * the same time because when the GUI thread sets up the data, the drawer has not started yet, or
 * has exited and been joined by the GUI.
 */
// microseconds
int const MAX_EVENT_REPORT_TIME = 20000;

// event identifier for identifying an event that occurs during a poll.  It doesn't matter what this
// value is, it just has to be unique among all the other sensors the program receives events for.
int const EVENT_TYPE_ACCELEROMETER = 462;

ASensorManager *sensorManager = nullptr;
ASensor const *sensor = nullptr;
ASensorEventQueue *eventQueue = nullptr;
ALooper *looper = nullptr;
JNIEnv *gEnv = nullptr;
std::unique_ptr<Graphics> graphics;

extern "C" JNIEXPORT jstring JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_MainActivity_initPipeline(
        JNIEnv *env,
        jobject thisptr,
        jboolean useVulkan,
        jobject drawingSurface,
        jobject assetManager)
{
    gEnv = env;
    sensorManager = ASensorManager_getInstance();
    sensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    if (sensor == nullptr) {
        return env->NewStringUTF("Accelerometer is not present");
    }

    looper = ALooper_forThread();
    if (looper == nullptr) {
        looper = ALooper_prepare(0);
    }

    if (looper == nullptr) {
        return env->NewStringUTF("Could not get looper.");
    }

    eventQueue = ASensorManager_createEventQueue(sensorManager, looper, EVENT_TYPE_ACCELEROMETER, nullptr, nullptr);
    if (eventQueue == nullptr) {
        return env->NewStringUTF("Could not create event queue for sensor");
    }

    ANativeWindow *window = ANativeWindow_fromSurface(env, drawingSurface);
    if (window == nullptr) {
        return env->NewStringUTF("Unable to acquire window from surface.");
    }

    AAssetManager *manager = AAssetManager_fromJava(env, assetManager);
    setAssetManager(manager);

    try {
        stopDrawing.store(false);
        if (useVulkan) {
            Graphics *g = new GraphicsVulkan(window);
            graphics.reset(g);
        } else {
            Graphics *g = new GraphicsGL(window);
            graphics.reset(g);
        }
        graphics->init(window);
    } catch (std::runtime_error &e) {
        graphics->cleanup();
        graphics.reset();
        return env->NewStringUTF(e.what());
    }

    return env->NewStringUTF("");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_draw(
        JNIEnv *env,
        jobject thisptr)
{
    if (stopDrawing.load()) {
        // There is nothing to do.  Just return, no results, no error.
        return env->NewStringUTF("");
    }

    gEnv = env;
    try {
        graphics->initThread();
        int rc = ASensorEventQueue_enableSensor(eventQueue, sensor);
        if (rc < 0) {
            return env->NewStringUTF("error: Could not enable sensor");
        }
        int minDelay = ASensor_getMinDelay(sensor);
        minDelay = std::max(minDelay, MAX_EVENT_REPORT_TIME);

        rc = ASensorEventQueue_setEventRate(eventQueue, sensor, minDelay);
        if (rc < 0) {
            ASensorEventQueue_disableSensor(eventQueue, sensor);
            graphics->cleanupThread();
            return env->NewStringUTF("error: Could not set event rate");
        }
        std::vector<std::string> results;
        while (!stopDrawing.load()) {
            timeval tv = {0, 1000};
            select(0, nullptr, nullptr, nullptr, &tv);
            rc = ASensorEventQueue_hasEvents(eventQueue);
            if (rc > 0) {
                std::vector<ASensorEvent> events;
                events.resize(100);
                ssize_t nbrEvents = ASensorEventQueue_getEvents(eventQueue, events.data(), events.size());
                if (nbrEvents < 0) {
                    // an error has occurred
                    ASensorEventQueue_disableSensor(eventQueue, sensor);
                    graphics->cleanupThread();
                    return env->NewStringUTF("error: Error on retrieving sensor events.");
                }

                float x = 0;
                float y = 0;
                float z = 0;
                for (int i = 0; i < nbrEvents; i++) {
                    x += events[i].acceleration.x;
                    y += events[i].acceleration.y;
                    z += events[i].acceleration.z;
                }
                graphics->updateAcceleration(x/nbrEvents, y/nbrEvents, z/nbrEvents);

            }
            bool drawingNecessary = graphics->updateData();
            if (drawingNecessary) {
                graphics->drawFrame();
            }
        }
    } catch (std::runtime_error &e) {
        // no need to draw another frame - we are failing.
        ASensorEventQueue_disableSensor(eventQueue, sensor);
        graphics->cleanupThread();
        return env->NewStringUTF((std::string("error: ") + e.what()).c_str());
    }

    graphics->cleanupThread();
    ASensorEventQueue_disableSensor(eventQueue, sensor);
    return env->NewStringUTF("");
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_MainActivity_tellDrawerStop(
        JNIEnv *env,
        jobject thisptr)
{
    stopDrawing.store(true);
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_MainActivity_destroyNativeSurface(
        JNIEnv *env,
        jobject thisptr)
{
    graphics->cleanup();
}

std::vector<char> getTextImage(std::string text, uint32_t &width, uint32_t &height, uint32_t &channels) {
    jclass imageLoaderClass = gEnv->FindClass("com/quasar/cerulean/amazinglabyrinth/TextImageLoader");
    jmethodID mid = gEnv->GetMethodID(imageLoaderClass, "<init>", "(Ljava/lang/String;)V");
    if (mid == 0) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jstring jtext = gEnv->NewStringUTF(text.c_str());
    jobject imageLoader = gEnv->NewObject(imageLoaderClass, mid, jtext);

    mid = gEnv->GetMethodID(imageLoaderClass, "getImageSize", "()I");
    if (mid == 0) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jsize = gEnv->CallIntMethod(imageLoader, mid);

    mid = gEnv->GetMethodID(imageLoaderClass, "getImageWidth", "()I");
    if (mid == 0) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jwidth = gEnv->CallIntMethod(imageLoader, mid);
    width = static_cast<uint32_t> (jwidth);

    mid = gEnv->GetMethodID(imageLoaderClass, "getImageHeight", "()I");
    if (mid == 0) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    jint jheight = gEnv->CallIntMethod(imageLoader, mid);
    height = static_cast<uint32_t> (jheight);

    jbyteArray jimageData = gEnv->NewByteArray(jsize);
    mid = gEnv->GetMethodID(imageLoaderClass, "getImageData", "([B)V");
    if (mid == 0) {
        throw (std::runtime_error("Could not find method in fetching the image for text"));
    }
    gEnv->CallVoidMethod(imageLoader, mid, jimageData);

    std::vector<char> imageData;
    size_t size = static_cast<size_t> (jsize);
    imageData.resize(size);
    jbyte *bytes = gEnv->GetByteArrayElements(jimageData, nullptr);
    memcpy(imageData.data(), bytes, size);
    gEnv->ReleaseByteArrayElements(jimageData, bytes, JNI_ABORT);

    channels = 4;
    return imageData;
}