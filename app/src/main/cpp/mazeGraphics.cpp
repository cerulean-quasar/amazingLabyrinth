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

#include "mazeGraphics.hpp"

void LevelSequence::setViewLightingSource() {
    m_viewLightingSource = glm::lookAt(m_lightingSource, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LevelSequence::setView() {
    /* glm::lookAt takes the eye position, the center position, and the up axis as parameters */
    m_view = getViewMatrix();
}

void LevelSequence::setLightingSource() {
    m_lightingSource = glm::vec3(0.0f, 0.0f, 1.28f/*0.01-1.28*/);
}

void LevelSequence::updateAcceleration(float x, float y, float z) {
    if (m_levelStarter.get() != nullptr) {
        m_levelStarter->updateAcceleration(x, y, z);
    } else {
        m_level->updateAcceleration(x, y, z);
    }
}

void LevelSequence::initializeLevelData(std::shared_ptr<Level> const &level, DrawObjectTable &staticObjsData,
                                        DrawObjectTable &dynObjsData, TextureMap &textures) {
    staticObjsData.clear();
    dynObjsData.clear();
    textures.clear();
    level->updateStaticDrawObjects(staticObjsData, textures);
    bool texturesChanged;
    level->updateDynamicDrawObjects(dynObjsData, textures, texturesChanged);
    addTextures(textures);
    addObjects(staticObjsData, textures);
    addObjects(dynObjsData, textures);
    m_texturesChanged = true;
}

void LevelSequence::addObjects(DrawObjectTable &objs, TextureMap &textures) {
    for (auto &obj : objs) {
        obj.second = createObject(obj.first, textures);
    }
}

void LevelSequence::addTextures(TextureMap &textures) {
    for (auto it = textures.begin(); it != textures.end(); it++) {
        if (it->second.get() == nullptr) {
            it->second = createTexture(it->first);
        }
    }
}

void LevelSequence::initializeLevelTracker() {
    if (m_levelTracker != nullptr) {
        return;
    }

    setView();
    updatePerspectiveMatrix(m_surfaceWidth, m_surfaceHeight);

    m_levelTracker = std::make_shared<LevelTracker>(m_gameRequester,
        getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight), m_view);

    setLightingSource();
    setViewLightingSource();

    m_levelTracker->setLevel(m_levelName);
    m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
    m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);

    float x, y;
    m_level->getLevelFinisherCenter(x, y);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y,
            getPerspectiveMatrixForLevel(m_surfaceWidth, m_surfaceHeight), m_view);

    setupLightingSourceBuffer();

    if (m_levelStarter != nullptr && ! m_levelStarter->isFinished()) {
        initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                            m_levelStarterDynObjsData, m_texturesLevelStarter);
    }
    initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);
}

bool LevelSequence::updateData() {
    initializeLevelTracker();

    bool drawingNecessary = false;

    if (m_level->isFinished() || m_levelFinisher->isUnveiling()) {
        if (m_levelFinisher->isUnveiling()) {
            if (m_levelFinisher->isDone()) {
                m_levelFinisherObjsData.clear();
                m_texturesLevelFinisher.clear();
                float x, y;
                m_level->getLevelFinisherCenter(x, y);
                m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y, m_proj, m_view);
                m_levelStarter->start();
                return false;
            }
        } else {
            if (m_levelFinisher->isDone()) {

                m_levelTracker->gotoNextLevel();
                m_levelGroupFcns = m_levelTracker->getLevelGroupFcns();

                m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);
                initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                                    m_levelStarterDynObjsData, m_texturesLevelStarter);

                m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
                initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

                m_levelFinisher->unveilNewLevel();
                return false;
            }
        }

        drawingNecessary = m_levelFinisher->updateDrawObjects(m_levelFinisherObjsData,
                                                              m_texturesLevelFinisher,
                                                              m_texturesChanged);
        if (!drawingNecessary) {
            return false;
        }

        updateLevelData(m_levelFinisherObjsData, m_texturesLevelFinisher);
    } else if (m_levelStarter != nullptr) {
        drawingNecessary = m_levelStarter->updateData();

        if (m_levelStarter->isFinished()) {
            m_levelStarter.reset();
            m_levelStarterDynObjsData.clear();
            m_levelStarterStaticObjsData.clear();
            m_texturesLevelStarter.clear();
            m_level->start();
            return false;
        }

        if (drawingNecessary) {
            m_levelStarter->updateDynamicDrawObjects(m_levelStarterDynObjsData,
                                                     m_texturesLevelStarter, m_texturesChanged);
            updateLevelData(m_levelStarterDynObjsData, m_texturesLevelStarter);
        }
    } else {
        drawingNecessary = m_level->updateData();

        if (drawingNecessary) {
            m_level->updateDynamicDrawObjects(m_dynObjsData, m_texturesLevel, m_texturesChanged);
            updateLevelData(m_dynObjsData, m_texturesLevel);
        }
    }

    return drawingNecessary;
}

void LevelSequence::changeLevel(size_t level) {
    m_levelTracker->setLevel(level);
    m_levelGroupFcns = m_levelTracker->getLevelGroupFcns();
    m_levelStarter = m_levelGroupFcns.getStarterFcn(*m_levelTracker);
    initializeLevelData(m_levelStarter, m_levelStarterStaticObjsData,
                        m_levelStarterDynObjsData, m_texturesLevelStarter);

    m_level = m_levelGroupFcns.getLevelFcn(*m_levelTracker);
    initializeLevelData(m_level, m_staticObjsData, m_dynObjsData, m_texturesLevel);

    m_levelFinisherObjsData.clear();
    m_texturesLevelFinisher.clear();

    float x, y;
    m_level->getLevelFinisherCenter(x, y);
    m_levelFinisher = m_levelGroupFcns.getFinisherFcn(*m_levelTracker, x, y, m_proj, m_view);

    m_levelStarter->start();
}