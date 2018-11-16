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

#ifdef CQ_ENABLE_VULKAN
#include "graphicsVulkan.hpp"
#endif

/* Used to communicate between the gui thread and the drawing thread.  When the GUI thread wants
 * the drawer to stop drawing, cleanup, and exit, it sets this value to true.  The GUI will set
 * this value to false before starting a new drawer.
 */
std::atomic<bool> stopDrawing(false);

/*
 * The following global is used for calling back into java.  It is only accessed by one thread:
 * the thread running the game.
 */
JNIEnv *gEnv = nullptr;

class Sensors {
public:
    struct AccelerationEvent {
        float x;
        float y;
        float z;
    };

    Sensors() {
        initSensors();
    }

    inline bool hasEvents() {
        return ASensorEventQueue_hasEvents(eventQueue) > 0;
    }

    inline std::vector<AccelerationEvent> getEvents() {
        std::vector<ASensorEvent> events;
        events.resize(100);
        ssize_t nbrEvents = ASensorEventQueue_getEvents(eventQueue, events.data(), events.size());
        if (nbrEvents < 0) {
            // an error has occurred
            throw std::runtime_error("Error on retrieving sensor events.");
        }

        std::vector<AccelerationEvent> avector;
        for (int i = 0; i < nbrEvents; i++) {
            AccelerationEvent a{
                    events[i].acceleration.x,
                    events[i].acceleration.y,
                    events[i].acceleration.z};
            avector.push_back(a);
        }

        return avector;
    }

    ~Sensors(){
        ASensorEventQueue_disableSensor(eventQueue, sensor);
        ASensorManager_destroyEventQueue(sensorManager, eventQueue);
    }
private:
// microseconds
    static int const MAX_EVENT_REPORT_TIME;

// event identifier for identifying an event that occurs during a poll.  It doesn't matter what this
// value is, it just has to be unique among all the other sensors the program receives events for.
    static int const EVENT_TYPE_ACCELEROMETER;

    ASensorManager *sensorManager = nullptr;
    ASensor const *sensor = nullptr;
    ASensorEventQueue *eventQueue = nullptr;
    ALooper *looper = nullptr;

    void initSensors();
};

int const Sensors::MAX_EVENT_REPORT_TIME = 20000;
int const Sensors::EVENT_TYPE_ACCELEROMETER = 462;

void Sensors::initSensors() {
    sensorManager = ASensorManager_getInstance();
    sensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    if (sensor == nullptr) {
        // TODO: use a flick gesture instead?
        throw std::runtime_error("Accelerometer not present.");
    }

    looper = ALooper_forThread();
    if (looper == nullptr) {
        looper = ALooper_prepare(0);
    }

    if (looper == nullptr) {
        throw std::runtime_error("Could not initialize looper.");
    }

    eventQueue = ASensorManager_createEventQueue(sensorManager, looper, EVENT_TYPE_ACCELEROMETER, nullptr, nullptr);

    int rc = ASensorEventQueue_enableSensor(eventQueue, sensor);
    if (rc < 0) {
        throw std::runtime_error("Could not enable sensor");
    }
    int minDelay = ASensor_getMinDelay(sensor);
    minDelay = std::max(minDelay, MAX_EVENT_REPORT_TIME);

    rc = ASensorEventQueue_setEventRate(eventQueue, sensor, minDelay);
    if (rc < 0) {
        ASensorEventQueue_disableSensor(eventQueue, sensor);
        throw std::runtime_error("Could not set event rate");
    }
}

void draw(std::unique_ptr<Graphics> const &graphics)
{
    Sensors sensor;
    while (!stopDrawing.load()) {
        timeval tv = {0, 1000};
        select(0, nullptr, nullptr, nullptr, &tv);
        if (sensor.hasEvents()) {
            std::vector<Sensors::AccelerationEvent> events = sensor.getEvents();

            float x = 0;
            float y = 0;
            float z = 0;
            for (auto const & event : events) {
                x += event.x;
                y += event.y;
                z += event.z;
            }
            size_t nbrEvents = events.size();
            graphics->updateAcceleration(x/nbrEvents, y/nbrEvents, z/nbrEvents);

        }
        bool drawingNecessary = graphics->updateData();
        if (drawingNecessary) {
            graphics->drawFrame();
        }
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_Draw_startGame(
        JNIEnv *env,
        jobject thisptr,
        jobject drawingSurface,
        jobject assetManager,
        jint level)
{
    gEnv = env;
    if (level < 0 || ! LevelTracker::validLevel(static_cast<uint32_t>(level))) {
        return env->NewStringUTF("Invalid level");
    }

    ANativeWindow *window = ANativeWindow_fromSurface(env, drawingSurface);
    if (window == nullptr) {
        return env->NewStringUTF("Unable to acquire window from surface.");
    }

    AAssetManager *manager = AAssetManager_fromJava(env, assetManager);
    setAssetManager(manager);

    stopDrawing.store(false);

    std::unique_ptr<Graphics> graphics;
    bool useGL = false;
#ifdef CQ_ENABLE_VULKAN
    try {
        graphics.reset(new GraphicsVulkan(window, level));
    } catch (std::runtime_error &e) {
        useGL = true;
    }
#else
    useGL = true;
#endif

    try {
        if (useGL) {
            graphics.reset(new GraphicsGL(window, level));
        }
    } catch (std::runtime_error &e) {
        return env->NewStringUTF(e.what());
    }

    try {
        draw(graphics);
    } catch (std::runtime_error &e) {
        return env->NewStringUTF(e.what());
    }

    return env->NewStringUTF("");
}

extern "C" JNIEXPORT void JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_MainActivity_tellDrawerStop(
        JNIEnv *env,
        jobject thisptr)
{
    stopDrawing.store(true);
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_quasar_cerulean_amazinglabyrinth_MainActivity_getLevelList(
        JNIEnv *env,
        jobject thisptr)
{
    std::vector<std::string> levels = LevelTracker::getLevelDescriptions();
    jobjectArray ret = (jobjectArray)env->NewObjectArray(levels.size(), env->FindClass("java/lang/String"), nullptr);
    for (int i = 0; i < levels.size(); i++) {
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(levels[i].c_str()));
    }

    return ret;
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

