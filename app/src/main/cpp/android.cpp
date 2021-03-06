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
#include "android_native_app_glue.h"
#include <string>
#include <asm/fcntl.h>
#include <istream>
#include <vector>

#include "android.hpp"

constexpr int Sensors::MAX_EVENT_REPORT_TIME;

void Sensors::initSensors(std::bitset<3> inWhichSensors) {
    m_sensorManager = ASensorManager_getInstance();
    if (inWhichSensors.test(LINEAR_ACCELERATION_SENSOR)) {
        m_sensorLinearAcceleration = ASensorManager_getDefaultSensor(m_sensorManager,
                                                                     ASENSOR_TYPE_LINEAR_ACCELERATION);
        if (m_sensorLinearAcceleration == nullptr) {
            // should not happen
            throw std::runtime_error("Linear acceleration sensor not present.");
        }
    }

    if (inWhichSensors.test(GRAVITY_SENSOR)) {
        m_sensorGravity = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_GRAVITY);
        if (m_sensorGravity == nullptr) {
            // should not happen
            throw std::runtime_error("Gravity sensor not present.");
        }
    }

    if (inWhichSensors.test(ACCELEROMETER_SENSOR)) {
        m_sensorAccelerometer = ASensorManager_getDefaultSensor(m_sensorManager,
                                                                ASENSOR_TYPE_ACCELEROMETER);
        if (m_sensorAccelerometer == nullptr) {
            // should not happen
            throw std::runtime_error("Accelerometer not present.");
        }
    }

    m_looper = ALooper_forThread();
    if (m_looper == nullptr) {
        m_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }

    if (m_looper == nullptr) {
        throw std::runtime_error("Could not initialize looper.");
    }

    m_eventQueueLinearAcceleration = initializeSensor(m_sensorLinearAcceleration, EVENT_TYPE_LINEAR_ACCELERATION);
    m_eventQueueGravity = initializeSensor(m_sensorGravity, EVENT_TYPE_GRAVITY);
    m_eventQueueAccelerometer = initializeSensor(m_sensorAccelerometer, EVENT_TYPE_ACCELEROMETER);
}

ASensorEventQueue *Sensors::initializeSensor(ASensor const *sensor, int eventType) {
    if (sensor == nullptr) {
        return nullptr;
    }

    ASensorEventQueue *eventQueue = ASensorManager_createEventQueue(m_sensorManager, m_looper,
                                                                    eventType, nullptr, nullptr);

    int rc = ASensorEventQueue_enableSensor(eventQueue, sensor);
    if (rc < 0) {
        ASensorManager_destroyEventQueue(m_sensorManager, eventQueue);
        destroyResources();
        throw std::runtime_error("Could not enable sensor");
    }
    int minDelay = ASensor_getMinDelay(sensor);
    minDelay = std::max(minDelay, MAX_EVENT_REPORT_TIME);

    rc = ASensorEventQueue_setEventRate(eventQueue, sensor, minDelay);
    if (rc < 0) {
        ASensorEventQueue_disableSensor(eventQueue, sensor);
        ASensorManager_destroyEventQueue(m_sensorManager, eventQueue);
        destroyResources();
        throw std::runtime_error("Could not set event rate");
    }

    return eventQueue;
}

std::unique_ptr<AAsset> AssetManagerWrapper::getAsset(std::string const &path) {
    AAsset *asset = AAssetManager_open(manager, path.c_str(), O_RDONLY);
    if (asset == nullptr) {
        throw std::runtime_error(std::string("File not found: ") + path);
    }
    return std::unique_ptr<AAsset>(asset);
}

AssetStreambuf::int_type AssetStreambuf::underflow() {
    int bytesRead = AAsset_read(asset.get(), buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        return traits_type::eof();
    }

    setg(buffer, buffer, buffer + bytesRead);
    return traits_type::to_int_type(buffer[0]);
}

std::streampos AssetStreambuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
    if (which != std::ios_base::in) {
        return -1;
    }
    off64_t offset = off;
    int whence;
    switch (way) {
        case std::ios_base::beg :
            whence = SEEK_SET;
            break;
        case std::ios_base::cur :
            whence = SEEK_CUR;
            break;
        case std::ios_base::end :
            whence = SEEK_END;
            break;
        default:
            return -1;
    }
    offset = AAsset_seek64(asset.get(), offset, whence);

    return offset;
}

std::streampos AssetStreambuf::seekpos(std::streampos pos, std::ios_base::openmode which) {
    return seekoff(pos, std::ios_base::beg, which);
}
