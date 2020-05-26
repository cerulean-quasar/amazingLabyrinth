/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of Amazing Labyrinth.
 *
 *  Amazing Labyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Amazing Labyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Amazing Labyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef AMAZING_LABYRINTH_DRAWER_HPP
#define AMAZING_LABYRINTH_DRAWER_HPP

#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include <bitset>
#include <boost/optional.hpp>
#include "mazeGraphics.hpp"
#include "common.hpp"
#include "android.hpp"

class DrawEvent {
public:
    enum evtype {
        stopDrawing,
        surfaceChanged,
        levelChanged,
        saveLevelData,
        drag,
        dragEnded,
        tap
    };

    // returns true if the surface needs redrawing after this event.
    virtual bool operator() (std::unique_ptr<Graphics> &diceGraphics) = 0;

    virtual evtype type() = 0;
    virtual ~DrawEvent() = default;
};

class StopDrawingEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        graphics->saveLevelData();
        return false;
    }

    evtype type() override { return stopDrawing; }

    ~StopDrawingEvent() override = default;
};

class SurfaceChangedEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        // giggle the device so that when the swapchain is recreated, it gets the correct width and
        // height.  Also updateData must be called before the first draw event because some
        // initialization is delayed till then.
        bool needsRedraw = graphics->updateData(true);
        graphics->drawFrame();

        graphics->changeRotationAngle(m_rotationAngle);
        graphics->recreateSwapChain(m_width, m_height);

        // redraw the frame right away and then return false because this event means that we
        // should redraw immediately and not wait for some number of events to come up before we
        // do the redraw.
        graphics->drawFrame();
        return false;
    }

    evtype type() override { return surfaceChanged; }

    SurfaceChangedEvent(uint32_t width, uint32_t height, float rotationAngle)
            : m_width(width),
              m_height(height),
              m_rotationAngle(rotationAngle)
    {}

    ~SurfaceChangedEvent() override = default;
private:
    uint32_t m_width;
    uint32_t m_height;
    float m_rotationAngle;
};

class LevelChangedEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        graphics->changeLevel(m_level);
        return true;
    }

    evtype type() override { return levelChanged; }

    LevelChangedEvent(uint32_t inLevel)
            : m_level{inLevel} {
    }

    ~LevelChangedEvent() override = default;
private:
    uint32_t m_level;
};

class DragEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        return graphics->drag(m_startX, m_startY, m_distanceX, m_distanceY);
    }

    evtype type() override { return levelChanged; }

    DragEvent(float startX, float startY, float distanceX, float distanceY)
            : m_startX{startX},
            m_startY{startY},
            m_distanceX{distanceX},
            m_distanceY{distanceY}
    {
    }

    ~DragEvent() override = default;
private:
    float m_startX;
    float m_startY;
    float m_distanceX;
    float m_distanceY;
};

class DragEndedEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        return graphics->dragEnded(m_x, m_y);
    }

    evtype type() override { return levelChanged; }

    DragEndedEvent(float x, float y)
            : m_x{x},
              m_y{y}
    {
    }

    ~DragEndedEvent() override = default;
private:
    float m_x;
    float m_y;
};

class TapEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        return graphics->tap(m_x, m_y);
    }

    evtype type() override { return levelChanged; }

    TapEvent(float x, float y)
            : m_x{x},
              m_y{y}
    {
    }

    ~TapEvent() override = default;
private:
    float m_x;
    float m_y;
};

class SaveLevelDataEvent : public DrawEvent {
public:
    bool operator() (std::unique_ptr<Graphics> &graphics) override {
        graphics->saveLevelData();
        return true;
    }

    evtype type() override { return saveLevelData; }

    SaveLevelDataEvent() {
    }

    ~SaveLevelDataEvent() override = default;
};

/* Used to communicate between the gui thread and the drawing thread.
 */
class GameSendChannel {
private:
    std::mutex m_eventLock;
    std::condition_variable m_eventConditionVariable;

    std::queue<std::shared_ptr<DrawEvent>> m_drawEventQueue;
    bool m_stopDrawing;

public:
    GameSendChannel()
            : m_stopDrawing{false}
    {}

    std::shared_ptr<DrawEvent> getEventNoWait();
    std::shared_ptr<DrawEvent> getEvent();

    // blocks on sending event
    void sendEvent(std::shared_ptr<DrawEvent> const &event);
    void sendStopDrawingEvent();
    void clearQueue();
};

GameSendChannel &gameFromGuiChannel();

class GameWorker {
public:
    GameWorker(std::shared_ptr<WindowType> inSurface,
               GameRequesterCreator inRequesterCreator,
               bool inUseGravity,
               bool inUseLegacy,
               float rotationAngle)
            : m_whichSensors{},
              m_tryVulkan{!inUseLegacy},
              m_graphics{}
    {
        std::bitset<3> whichSensors = Sensors::hasWhichSensors();
        if (inUseGravity) {
            // The user selected to not use gravity.  We need the Linear Acceleration Sensor
            // for this feature.
            if (whichSensors.test(Sensors::ACCELEROMETER_SENSOR)) {
                m_whichSensors.set(Sensors::ACCELEROMETER_SENSOR);
            }
        }

        std::string error = std::move(initGraphics(std::move(inSurface), inRequesterCreator, rotationAngle));
        m_graphics->sendGraphicsDescription(whichSensors.test(Sensors::ACCELEROMETER_SENSOR), error);
    }

    void drawingLoop();
private:
    static constexpr uint32_t m_maxEventsBeforeRedraw = 128;

    std::bitset<3> m_whichSensors;
    bool m_tryVulkan;
    std::unique_ptr<Graphics> m_graphics;
    static uint32_t constexpr m_maxIterationsIdle = 20000;

    std::string initGraphics(std::shared_ptr<WindowType> surface, GameRequesterCreator requesterCreator,
            float rotationAngle);
};

#endif // AMAZING_LABYRINTH_DRAWER_HPP
