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

#include "rotatablePassage.hpp"
#include "../generatedMazeAlgorithms.hpp"

void RotatablePassage::initSetGameBoard(uint32_t nbrTilesX, GeneratedMazeBoard::Mode mode) {
    uint32_t nbrTilesY = static_cast<uint32_t>(std::floor(nbrTilesX/m_width * m_height));

    // nbrTilesX+2 to get the tile size because we want a border of at least one tile around the
    // edge.
    m_gameBoard.initialize(m_width/(nbrTilesX+2), glm::vec3{0.0f, 0.0f, m_mazeFloorZ}, nbrTilesY, nbrTilesX, 0, 0);

    // generate the maze
    GeneratedMazeBoard mazeBoard{nbrTilesY, nbrTilesX, mode};

    m_ballRow = mazeBoard.rowStart();
    m_ballCol = mazeBoard.colStart();
    m_endRow = mazeBoard.rowEnd();
    m_endCol = mazeBoard.colEnd();

    // add the components into the game board
    for (uint32_t i = 0; i < nbrTilesY; i++) {
        for (uint32_t j = 0; j < nbrTilesX; j++) {
            Cell const &cell = mazeBoard.getCell(i, j);
            bool right = cell.rightWallExists();
            bool top = cell.topWallExists();
            bool left = cell.leftWallExists();
            bool bottom = cell.bottomWallExists();

            uint32_t nbrWalls = 0;
            if (right) {
                nbrWalls++;
            }
            if (top) {
                nbrWalls++;
            }
            if (left) {
                nbrWalls++;
            }
            if (bottom) {
                nbrWalls++;
            }

            float angle = m_randomNumbers.getUInt(0, 3) * glm::radians(90.0f);
            Component::ComponentType componentType = Component::ComponentType::straight;
            if (nbrWalls == 3) {
                // dead end
                componentType = Component::ComponentType::deadEnd;
            } else if (nbrWalls == 2) {
                if ((right && top) || (top && left) || (left && bottom) || (bottom && right)) {
                    // turn junction
                    componentType = Component::ComponentType::turn;
                } else if ((right && left) || (bottom && top)) {
                    // straight junction
                    componentType = Component::ComponentType::straight;
                }
            } else if (nbrWalls == 1) {
                // t-junction
                componentType = Component::ComponentType::tjunction;
            } else if (nbrWalls == 0) {
                // cross junction
                componentType = Component::ComponentType::crossjunction;
            }
            uint32_t placementIndex = m_components[componentType]->add(i, j, angle);
            m_gameBoard.block(i, j).setComponent(m_components[componentType], placementIndex);
        }
    }

    m_initDone = true;
}

bool RotatablePassage::updateData() {
    if (!m_initDone) {
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(
            currentTime - m_prevTime).count();
    float timeDiffTotal = timeDiff;
    if (timeDiff < m_floatErrorAmount) {
        return false;
    }
    m_prevTime = currentTime;

    glm::vec3 position = m_ball.position;
    glm::vec3 prevPosition = position;
    m_ball.velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
    if (glm::length(m_ball.velocity) < m_floatErrorAmount) {
        return false;
    }

    uint32_t nbrComputations = 0;
    bool drawingNecessary_ = false;
    glm::vec3 posFromCenter = position - m_gameBoard.position(m_ballRow, m_ballCol);

    Component::checkForNextWallFunc checkforNextWall{
            [&](Component::CellWall wall1, Component::CellWall wall2)
                    -> std::pair<bool, bool> {
                return m_gameBoard.checkforNextWall(wall1, wall2, m_ballRow, m_ballCol);
            }
    };
    auto &block = m_gameBoard.block(m_ballRow, m_ballCol);
    auto walls = block.component()->moveBallInCell(
            block.placementIndex(), posFromCenter, timeDiff, m_ball.velocity, ballRadius(),
            checkforNextWall);

    position = m_gameBoard.position(m_ballRow, m_ballCol) + posFromCenter;
    if (walls.first == Component::noWall && walls.second == Component::noWall) {
        m_ball.position = position;
        updateRotation(timeDiffTotal);
        return drawingNecessary();
    }

    if (walls.first == Component::CellWall::wallLeft ||
        walls.second == Component::CellWall::wallLeft) {
        m_ballCol--;
    } else if (walls.first == Component::CellWall::wallRight ||
               walls.second == Component::CellWall::wallRight) {
        m_ballCol++;
    }
    if (walls.first == Component::CellWall::wallDown ||
        walls.second == Component::CellWall::wallDown) {
        m_ballRow--;
    } else if (walls.first == Component::CellWall::wallUp ||
               walls.second == Component::CellWall::wallUp) {
        m_ballRow++;
    }

    auto &nextBlock = m_gameBoard.block(m_ballRow, m_ballCol);
    if (m_ballRow == m_endRow && m_ballCol == m_endCol) {
        // winning condition.  Set m_finished and return that drawing is necessary.
        m_finished = true;
        m_ball.position = position;
        updateRotation(timeDiffTotal);
        return true;
    }

    block.component()->placement(block.placementIndex()).setLockedIntoPlace(false);
    nextBlock.component()->placement(nextBlock.placementIndex()).setLockedIntoPlace(true);

    m_ball.position = position;
    updateRotation(timeDiffTotal);
    return drawingNecessary();
}

bool RotatablePassage::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.empty()) {
        auto refs = addObjs(m_gameRequester, objs, textures, std::vector<std::string>{}, m_borderTextures);

        float scale = m_gameBoard.blockSize() / m_modelSize;

        // the left and right
        for (size_t i = 0; i < m_gameBoard.heightInTiles(); i++) {
            glm::vec3 pos = m_gameBoard.position(i, 0);
            pos.x -= m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                    glm::translate(glm::mat4{1.0f}, pos) *
                    glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos.x -= m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos = m_gameBoard.position(i, m_gameBoard.widthInTiles() - 1);
            pos.x += m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos.x += m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));
        }

        // the top and bottom
        for (size_t i = 0; i < m_gameBoard.widthInTiles(); i++) {
            glm::vec3 pos = m_gameBoard.position(0, i);
            pos.y -= m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos.y -= m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos = m_gameBoard.position(m_gameBoard.heightInTiles() - 1, i);
            pos.y += m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

            pos.y += m_gameBoard.blockSize();
            addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                                glm::translate(glm::mat4{1.0f}, pos) *
                                glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));
        }

        // the bottom left corner
        glm::vec3 pos = m_gameBoard.position(0, 0);
        pos.y -= m_gameBoard.blockSize();
        pos.x -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.x -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        // the bottom right corner
        pos = m_gameBoard.position(0, m_gameBoard.widthInTiles()-1);
        pos.y -= m_gameBoard.blockSize();
        pos.x += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.x += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        // the top right corner
        pos = m_gameBoard.position(m_gameBoard.heightInTiles() - 1, m_gameBoard.widthInTiles()-1);
        pos.y += m_gameBoard.blockSize();
        pos.x += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.x += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        // the top left corner
        pos = m_gameBoard.position(m_gameBoard.heightInTiles() - 1, 0);
        pos.y += m_gameBoard.blockSize();
        pos.x -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y += m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.x -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        pos.y -= m_gameBoard.blockSize();
        addModelMatrixToObj(m_randomNumbers, objs, refs, nullptr, 0,
                            glm::translate(glm::mat4{1.0f}, pos) *
                            glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));

        return true;
    }

    return false;
}

bool RotatablePassage::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures,
                                                bool &texturesChanged)
{
    float scale = m_gameBoard.blockSize() / m_modelSize;
    float scaleBall = scale / 2.0f;
    if (objs.empty()) {
        for (auto &component : m_components) {
            Component::ComponentType type = component->type();
            auto refs = addObjs(m_gameRequester, objs, textures, m_componentModels[type],
                                m_componentTextures[type]);
            component->setDynObjReferences(refs);

            size_t i = 0;
            for (auto placementIt = component->placementsBegin();
                 placementIt != component->placementsEnd();
                 placementIt++, i++) {
                auto pos = m_gameBoard.position(placementIt->row(), placementIt->col());
                addModelMatrixToObj(m_randomNumbers, objs, refs, component, i,
                                    glm::translate(glm::mat4{1.0f}, pos) *
                                    glm::rotate(glm::mat4{1.0f}, placementIt->rotationAngle(),
                                            glm::vec3{0.0f, 0.0f, 1.0f}) *
                                    glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale}));
            }
        }

        // the ball
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> v;
        auto obj = std::make_shared<DrawObject>();
        loadModel(m_gameRequester->getAssetStream(m_ballModel), v);
        std::swap(v.first, obj->vertices);
        std::swap(v.second, obj->indices);
        obj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_ballTextureName);
        textures.emplace(obj->texture, std::shared_ptr<TextureData>());
        obj->modelMatrices.push_back(
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{scaleBall, scaleBall, scaleBall}));
        m_objsReferenceBall = objs.size();
        objs.emplace_back(obj, std::shared_ptr<DrawObjectData>());
        texturesChanged = true;
    } else {
        std::vector<size_t> nbrModelMatrices;
        nbrModelMatrices.resize(objs.size(), 0);
        for (auto &component : m_components) {
            size_t i = 0;
            auto refs = component->dynObjReferences();
            for (auto placementIt = component->placementsBegin();
                 placementIt != component->placementsEnd();
                 placementIt++, i++)
            {
                auto pos = m_gameBoard.position(placementIt->row(), placementIt->col());
                auto ref = placementIt->dynObjReference();
                objs[ref].first->modelMatrices[nbrModelMatrices[ref]] =
                                    glm::translate(glm::mat4{1.0f}, pos) *
                                    glm::rotate(glm::mat4{1.0f}, placementIt->rotationAngle(),
                                            glm::vec3{0.0f, 0.0f, 1.0f}) *
                                    glm::scale(glm::mat4{1.0f}, glm::vec3{scale, scale, scale});
                nbrModelMatrices[ref]++;
            }
        }

        // the ball
        objs[m_objsReferenceBall].first->modelMatrices[0] =
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{scaleBall, scaleBall, scaleBall});

    }

    return true;
}
