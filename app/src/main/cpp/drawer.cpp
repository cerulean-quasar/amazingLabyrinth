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
#include "graphics.hpp"
#include "android.hpp"
#include "drawer.hpp"
#include "gameRequester.hpp"
#include "mazeGL.hpp"

#ifdef CQ_ENABLE_VULKAN
#include "mazeVulkan.hpp"
#endif

std::shared_ptr<DrawEvent> GameSendChannel::getEventNoWait() {
    // critical section
    std::unique_lock<std::mutex> lock(m_eventLock);

    if (m_stopDrawing) {
        m_stopDrawing = false;
        return std::make_shared<StopDrawingEvent>(StopDrawingEvent{});
    }

    if (m_drawEventQueue.empty()) {
        return std::shared_ptr<DrawEvent>();
    }

    std::shared_ptr<DrawEvent> event = m_drawEventQueue.front();
    m_drawEventQueue.pop();
    return event;
}

// blocks waiting for the next event.
std::shared_ptr<DrawEvent> GameSendChannel::getEvent() {
    // critical section
    std::unique_lock<std::mutex> lock(m_eventLock);

    if (m_stopDrawing) {
        m_stopDrawing = false;
        return std::make_shared<StopDrawingEvent>(StopDrawingEvent{});
    }

    if (m_drawEventQueue.empty()) {
        m_eventConditionVariable.wait(lock);
    }

    if (m_stopDrawing) {
        m_stopDrawing = false;
        return std::make_shared<StopDrawingEvent>(StopDrawingEvent{});
    }

    std::shared_ptr<DrawEvent> event = m_drawEventQueue.front();
    m_drawEventQueue.pop();
    return event;
}

void GameSendChannel::sendEvent(std::shared_ptr<DrawEvent> const &event) {
    std::unique_lock<std::mutex> lock(m_eventLock);

    m_drawEventQueue.push(event);
    m_eventConditionVariable.notify_one();
}

void GameSendChannel::sendStopDrawingEvent() {
    std::unique_lock<std::mutex> lock(m_eventLock);

    m_stopDrawing = true;
    m_eventConditionVariable.notify_one();
}

void GameSendChannel::clearQueue() {
    std::unique_lock<std::mutex> lock(m_eventLock);
    m_stopDrawing = false;

    while (!m_drawEventQueue.empty()) {
        m_drawEventQueue.pop();
    }
}

GameSendChannel &gameFromGuiChannel() {
    static GameSendChannel g_diceChannel{};

    return g_diceChannel;
}

std::string GameWorker::initGraphics(std::shared_ptr<WindowType> surface,
        GameRequesterCreator requesterCreator,
        float rotationAngle)
{
    std::string error;
#ifdef CQ_ENABLE_VULKAN
    if (m_tryVulkan) {
        try {
            m_graphics = std::make_unique<GraphicsVulkan>(surface, requesterCreator, rotationAngle);
        } catch (std::runtime_error &e) {
            error = std::string("Vulkan supported but not used due to: ") + e.what();
            m_tryVulkan = false;
        }
    }
#else
    m_tryVulkan = false;
#endif

    if (!m_tryVulkan) {
        m_graphics = std::make_unique<GraphicsGL>(std::move(surface), requesterCreator, rotationAngle);
    }

    return std::move(error);
}

void GameWorker::drawingLoop() {
    std::unique_ptr<Sensors> sensor;
    if (m_whichSensors.any()) {
        sensor = std::make_unique<Sensors>(m_whichSensors);
    }

    while (true) {
        if (sensor != nullptr && sensor->hasAccelerometerEvents()) {
            std::vector<Sensors::AccelerationEvent> events = sensor->getAccelerometerEvents();
            for (auto const &event : events) {
                m_graphics->updateAcceleration(event.x, event.y, event.z);
            }
        }

        // Some events are a lot cheaper than a redraw, so process several of them.
        uint32_t nbrRequireRedraw = 0;
        while (nbrRequireRedraw < m_maxEventsBeforeRedraw) {
            auto event = gameFromGuiChannel().getEventNoWait();
            if (event != nullptr) {
                switch (event->type()) {
                    case DrawEvent::stopDrawing:
                        // The main thread requested that we exit.  Run the event and then exit.
                        (*event)(m_graphics);
                        return;
                    case DrawEvent::surfaceChanged:
                    case DrawEvent::saveLevelData:
                    case DrawEvent::tiltMaze:
                    case DrawEvent::levelChanged:
                        if ((*event)(m_graphics)) {
                            nbrRequireRedraw++;
                        }
                        break;
                }
            } else {
                break;
            }
        }

        bool needsRedraw = m_graphics->updateData();
        if (needsRedraw || nbrRequireRedraw > 0) {
            m_graphics->drawFrame();
        } else {
            timeval tv = {0, 1000};
            select(0, nullptr, nullptr, nullptr, &tv);
        }
    }
}
