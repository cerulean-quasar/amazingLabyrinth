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

#include <array>

#include "fixedMaze.hpp"

glm::vec4 FixedMaze::getBackgroundColor() {
    return glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
}

void FixedMaze::updateAcceleration(float x, float y, float z) {
    m_ball.acceleration = glm::vec3{-x, -y, -z};
}

void boundsCheck(int32_t &cell, size_t size) {
    if (cell < 0) {
        cell = 0;
        return;
    }

    if (cell > size - 1) {
        cell = size - 1;
    }
}

size_t FixedMaze::getXCell(float x) {
    int32_t xcell = static_cast<int32_t>(std::floor((x + m_width/2)/m_width * m_rowWidth));

    // deal with floating point errors.
    if ((xcell * m_width / m_rowWidth - m_width / 2.0f - x) > 0) {
        xcell--;
    } else if (((xcell+1) * m_width / m_rowWidth - m_width / 2.0f - x) < 0) {
        xcell++;
    }

    boundsCheck(xcell, m_rowWidth);
    return static_cast<size_t>(xcell);
}

size_t FixedMaze::getYCell(float y) {
    int32_t ycell = static_cast<size_t>(std::floor((y + m_height/2)/m_height * m_rowHeight));

    // deal with floating point errors.
    if ((ycell * m_height / m_rowHeight - m_height / 2.0f - y) > 0) {
        ycell--;
    } else if (((ycell+1) * m_height / m_rowHeight - m_height / 2.0f - y) < 0) {
        ycell++;
    }

    boundsCheck(ycell, m_rowHeight);
    return static_cast<size_t>(ycell);
}

float FixedMaze::getZPos(float x, float y) {
    return getZPos(x, y, m_scaleBall * MODEL_BALL_SIZE/2.0f);
}

float FixedMaze::getZPos(float x, float y, float extend) {
    float maxZ = m_maxZ - MODEL_MAXZ;
    size_t xcellmin = getXCell(x - extend);
    size_t xcellmax = getXCell(x + extend);
    size_t ycellmin = getYCell(y - extend);
    size_t ycellmax = getYCell(y + extend);

    for (size_t i = xcellmin; i <= xcellmax; i++) {
        for (size_t j = ycellmin; j <= ycellmax; j++) {
            float z = getRawDepth(i, j);
            if (z <= m_maxZ - MODEL_MAXZ + m_floatErrorAmount) {
                // we are in the "hole", just return the min depth as is, no averaging.
                return z;
            } else  if (z > maxZ) {
                maxZ = z;
            }
        }
    }

    return maxZ;
}

float FixedMaze::getRawDepth(float x, float y) {
    size_t xcell = getXCell(x);
    size_t ycell = getYCell(y);

    return getRawDepth(xcell, ycell);
}
float FixedMaze::getRawDepth(size_t xcell, size_t ycell) {
    float z = m_depthMap[ycell * m_rowWidth + xcell];
    if (z > m_maxZ + MODEL_MAXZ || z < m_maxZ - MODEL_MAXZ) {
        // if we are outside of the range the model is supposed to be in for Z, then
        // assume the ball is in the "hole".  Return the special value: m_maxZ - MODEL_MAXZ
        // to indicate this.
        z = m_maxZ - MODEL_MAXZ;
    }

    return z;
}

glm::vec3 FixedMaze::getNormalAtPosition(float x, float y) {
    return m_normalMap[getYCell(y) * m_rowWidth +  getXCell(x)];
}

glm::vec3 FixedMaze::getParallelAcceleration() {
    glm::vec3 normal = getNormalAtPosition(m_ball.position.x, m_ball.position.y);
    glm::vec3 normalGravityComponent = glm::dot(normal, m_ball.acceleration) * normal;

    return m_ball.acceleration - normalGravityComponent;
}

void FixedMaze::moveBall(float timeDiff) {
    glm::vec3 position = m_ball.position;
    glm::vec3 velocity = m_ball.velocity + getParallelAcceleration() * timeDiff - m_viscosity * m_ball.velocity;

    glm::vec3 surfaceNormal = getNormalAtPosition(position.x, position.y);
    velocity -= glm::dot(velocity, surfaceNormal) * surfaceNormal;

    if (glm::length(velocity) < m_floatErrorAmount) {
        // velocity is too small.  Return.
        return;
    }

    while (timeDiff > 0.0f) {
        glm::vec3 nextPos = position + velocity * timeDiff;
        size_t xcell = getXCell(position.x);
        size_t ycell = getYCell(position.y);

        float deltax = nextPos.x - position.x;
        float deltay = nextPos.y - position.y;
        std::array<float, 4> fractionsTillWall =  { -1.0f, -1.0f, -1.0f, -1.0f };

        if (deltax > m_floatErrorAmount || deltax < -m_floatErrorAmount) {
            fractionsTillWall[0] = (xcell * m_width / m_rowWidth - m_width / 2.0f - position.x) / deltax;
            fractionsTillWall[1] = ((xcell + 1) * m_width / m_rowWidth - m_width/2.0f - position.x) / deltax;
        }

        if (deltay > m_floatErrorAmount ||  deltay < -m_floatErrorAmount) {
            fractionsTillWall[2] = (ycell * m_height / m_rowHeight - m_height / 2.0f - position.y) / deltay;
            fractionsTillWall[3] = ((ycell + 1) * m_height / m_rowHeight - m_height / 2.0f - position.y) / deltay;
        }

        size_t smallest = fractionsTillWall.size();
        size_t diagonal = fractionsTillWall.size();
        for (size_t i = 0; i < fractionsTillWall.size(); i++) {
            if (fractionsTillWall[i] < m_floatErrorAmount) {
                continue;
            }

            if (smallest >= fractionsTillWall.size()) {
                smallest = i;
            } else if (fractionsTillWall[i] == fractionsTillWall[smallest]) {
                diagonal = i;
            } else if (fractionsTillWall[i] < fractionsTillWall[smallest]) {
                smallest = i;
            }
        }

        if (smallest >= fractionsTillWall.size()) {
            // shouldn't happen
            return;
        }

        if (fractionsTillWall[smallest] >= 1.0f) {
            // moving within a cell - just move and break.
            position = nextPos;
            position.z = getZPos(position.x, position.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
            break;
        }

        if (smallest == 0 && xcell == 0) {
            // left wall
            position.x = -m_width/2.0f + m_scaleBall * MODEL_BALL_SIZE / 2.0f;
            position.z = getZPos(position.x, position.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
            timeDiff -= fractionsTillWall[smallest] * timeDiff;
            if (velocity.x < 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                    continue;
                } else {
                    velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                    break;
                }
            }
        }

        if (smallest == 1 && xcell == m_rowWidth - 1) {
            // right wall
            position.x = m_width/2.0f - m_scaleBall * MODEL_BALL_SIZE / 2.0f;
            position.z = getZPos(position.x, position.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
            timeDiff -= fractionsTillWall[smallest] * timeDiff;
            if (velocity.x > 0.0f) {
                if (m_bounce) {
                    velocity.x = -velocity.x;
                    continue;
                } else {
                    velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                    break;
                }
            }
        }

        if (smallest == 2 && ycell == 0) {
            // bottom wall
            position.y = -m_height/2.0f + m_scaleBall * MODEL_BALL_SIZE / 2.0f;
            position.z = getZPos(position.x, position.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
            timeDiff -= fractionsTillWall[smallest] * timeDiff;
            if (velocity.y < 0.0f) {
                if (m_bounce) {
                    velocity.y = -velocity.y;
                    continue;
                } else {
                    velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                    break;
                }
            }
        }

        if (smallest == 3 && ycell == m_rowHeight - 1) {
            // top wall
            position.y = m_height/2.0f - m_scaleBall * MODEL_BALL_SIZE / 2.0f;
            position.z = getZPos(position.x, position.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
            timeDiff -= fractionsTillWall[smallest] * timeDiff;
            if (velocity.y > 0.0f) {
                if (m_bounce) {
                    velocity.y = -velocity.y;
                    continue;
                } else {
                    velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                    break;
                }
            }
        }

        glm::vec3 newPos = position;
        auto adjustPos = [&] (size_t fractionIndex) -> void {
            switch (fractionIndex) {
                case 0:
                    newPos.x += (nextPos.x - position.x) * fractionsTillWall[fractionIndex] - m_floatErrorAmount;
                    break;
                case 1:
                    newPos.x += (nextPos.x - position.x) * fractionsTillWall[fractionIndex] + m_floatErrorAmount;
                    break;
                case 2:
                    newPos.y += (nextPos.y - position.y) * fractionsTillWall[fractionIndex] - m_floatErrorAmount;
                    break;
                case 3:
                    newPos.y += (nextPos.y - position.y) * fractionsTillWall[fractionIndex] + m_floatErrorAmount ;
                    break;
                default:
                    // shouldn't happen
                    return;
            }
        };

        adjustPos(smallest);

        if (diagonal < fractionsTillWall.size()) {
            adjustPos(diagonal);
        }

        glm::vec2 newPosXY{newPos.x, newPos.y};
        glm::vec2 positionXY{position.x, position.y};
        glm::vec2 nextPosXY{nextPos.x, nextPos.y};
        float timeInc = glm::length(newPosXY - positionXY) / glm::length(nextPosXY - positionXY) * timeDiff;

        newPos.z = getZPos(newPos.x, newPos.y) + m_scaleBall*MODEL_BALL_SIZE/2.0f;
        if (m_stopAtSteepSlope && newPos.z > position.z + m_scaleBall * MODEL_BALL_SIZE/10.0f) {
            if (m_bounce) {
                glm::vec3 normal = getNormalAtPosition(newPos.x, newPos.y);
                if (normal.z > 0.99999f) {
                    /* x and y components of the normal too small to compute the reflective
                     * velocity.  Just negate x and y velocity components so it goes back in the
                     * direction it came from.
                     */
                    velocity = glm::vec3{-velocity.x, -velocity.y, 0.0f};
                    position = newPos;
                    timeDiff -= timeInc;
                    continue;
                }
                glm::vec3 xyNormal = glm::normalize(glm::vec3{normal.x, normal.y, 0.0f});
                float velocityNormalToSurface = glm::dot(xyNormal, velocity);
                if (velocityNormalToSurface < 0.0f) {
                    velocity = velocity - 2.0f * velocityNormalToSurface * xyNormal;
                    position = newPos;
                    timeDiff -= timeInc;
                    continue;
                }
            } else {
                velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                position = newPos;
                timeDiff -= timeInc;
                break;
            }
        }
        position = newPos;
        timeDiff -= timeInc;
    }

    surfaceNormal = getNormalAtPosition(position.x, position.y);
    velocity -= glm::dot(velocity, surfaceNormal) * surfaceNormal;

    m_ball.velocity = velocity;
    m_ball.position = position;
}

bool FixedMaze::updateData() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    moveBall(timeDiff);

    if (glm::length(m_ball.velocity) > 0.0f) {
        glm::vec3 axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), m_ball.velocity);
        if (glm::length(axis) != 0) {
            float scaleFactor = 10.0f;
            glm::quat q = glm::angleAxis(glm::length(m_ball.velocity) * timeDiff * scaleFactor,
                                         glm::normalize(axis));

            m_ball.totalRotated = glm::normalize(q * m_ball.totalRotated);
        }
    }

    bool drawingNecessary = glm::length(m_ball.position - m_ball.prevPosition) > 0.005;
    if (drawingNecessary) {
        m_ball.prevPosition = m_ball.position;
    }
    return drawingNecessary;
}

bool FixedMaze::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.empty() && textures.empty()) {
        /* floor */
        //objs.emplace_back(m_testObj, nullptr);
        auto obj = m_worldMap.begin();
        obj->first->texture = m_testObj->texture;
        objs.emplace_back(obj->first, nullptr);

        textures.insert(
                std::make_pair(std::make_shared<TextureDescriptionDummy>(m_gameRequester),
                               m_testTexture));
        return true;
    }

    return false;
}

bool FixedMaze::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    texturesChanged = false;
    if (objs.empty()) {
        /* ball */
        std::shared_ptr<DrawObject> ballObj = std::make_shared<DrawObject>();
        ballObj->vertices = m_ballVertices;
        ballObj->indices = m_ballIndices;
        ballObj->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
                                                                    m_ballTextureName);
        textures.insert(std::make_pair(ballObj->texture, std::shared_ptr<TextureData>()));
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                    glm::mat4_cast(m_ball.totalRotated) *
                                    glm::scale(glm::mat4(1.0f),
                                               glm::vec3{m_scaleBall, m_scaleBall,
                                                         m_scaleBall});
        ballObj->modelMatrices.push_back(modelMatrixBall);
        objs.emplace_back(ballObj, nullptr);
        texturesChanged = true;
    } else {
        glm::mat4 modelMatrixBall = glm::translate(glm::mat4(1.0f), m_ball.position) *
                                    glm::mat4_cast(m_ball.totalRotated) *
                                    glm::scale(glm::mat4(1.0f),
                                               glm::vec3{m_scaleBall, m_scaleBall,
                                                         m_scaleBall});
        std::shared_ptr<DrawObject> ballObj = objs[0].first;
        ballObj->modelMatrices[0] = modelMatrixBall;

    }
    return true;
}

void FixedMaze::start() {
    m_prevTime = std::chrono::high_resolution_clock::now();
}

void FixedMaze::getLevelFinisherCenter(float &x, float &y) {
    x = 0.0f;
    y = 0.0f;
}

void FixedMaze::loadModels() {
    loadModel(m_gameRequester->getAssetStream(MODEL_BALL), m_ballVertices, m_ballIndices);
}

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester, float width, float height, float maxZ)
    : Level{inGameRequester, width, height, maxZ},
    m_scaleBall{width/10.0f/2.0f}
{
    init();
}

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
    std::shared_ptr<FixedMazeSaveData> sd,
    float width, float height, float maxZ)
    : Level{inGameRequester, width, height, maxZ},
    m_scaleBall{width/10.0f/2.0f}
{
    init();
}

void FixedMaze::init()
{
    m_prevTime = std::chrono::high_resolution_clock::now();
    m_stopAtSteepSlope = true;
    m_bounce = true;

    loadModels();

    auto worldObj = std::make_shared<DrawObject>();
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> verticesWithVertexNormals;
    loadModel(m_gameRequester->getAssetStream("models/mountainLandscape.obj"), worldObj->vertices,
            worldObj->indices, &verticesWithVertexNormals);
    worldObj->modelMatrices.push_back(
            glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_width/MODEL_WIDTH, m_height/MODEL_HEIGHT, 1.0f}) *
            glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
    worldObj->texture = nullptr;
    m_worldMap.emplace_back(worldObj, nullptr);

    auto worldObj2 = std::make_shared<DrawObject>();
    worldObj2->vertices = verticesWithVertexNormals.first;
    worldObj2->indices = verticesWithVertexNormals.second;
    worldObj2->modelMatrices = worldObj->modelMatrices;
    worldObj2->texture = nullptr;

    DrawObjectTable worldMap;
    worldMap.emplace_back(worldObj2, nullptr);

    m_rowWidth = static_cast<uint32_t>(std::floor(10.0f/m_scaleBall * m_width));
    m_testTexture = m_gameRequester->getDepthTexture(worldMap, m_width, m_height, m_rowWidth,
            m_depthMap, m_normalMap);
    m_rowHeight = m_depthMap.size()/m_rowWidth;

    m_testObj = std::make_shared<DrawObject>();
    m_testObj->texture = std::make_shared<TextureDescriptionDummy>(m_gameRequester);
    getQuad(m_testObj->vertices, m_testObj->indices);
    m_testObj->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_maxZ}) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3{m_width/2.0f, m_height/2.0f, 1.0f}));

    m_ball.position = {-m_width/2.0f + MODEL_BALL_SIZE*m_scaleBall/2.0f, -m_height/2.0f + MODEL_BALL_SIZE*m_scaleBall/2.0f, 0.0f};
    m_ball.position.z = getZPos(m_ball.position.x, m_ball.position.y) + m_scaleBall * MODEL_BALL_SIZE /2.0f;
}
