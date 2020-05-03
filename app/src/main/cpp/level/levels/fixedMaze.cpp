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
#include "../level.hpp"

bool notValid(glm::vec3 const &v) {
    if (v.x != v.x || v.y != v.y || v.z != v.z) {
        return true;
    }

    return false;
}

glm::vec4 FixedMaze::getBackgroundColor() {
    return glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
}

void boundsCheck(int32_t &cell, size_t size) {
    if (cell < 0) {
        cell = 0;
        return;
    }

    if (cell > static_cast<int32_t>(size) - 1) {
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
    return getZPos(x, y, ballRadius());
}

float FixedMaze::getZPos(float x, float y, float extend) {
    float maxZ = m_mazeFloorZ - MODEL_MAXZ;
    size_t xcellmin = getXCell(x - extend);
    size_t xcellmax = getXCell(x + extend);
    size_t ycellmin = getYCell(y - extend);
    size_t ycellmax = getYCell(y + extend);

    for (size_t i = xcellmin; i <= xcellmax; i++) {
        for (size_t j = ycellmin; j <= ycellmax; j++) {
            float z = getRawDepth(i, j);
            if (z <= m_mazeFloorZ - MODEL_MAXZ + m_floatErrorAmount) {
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
    if (z > m_mazeFloorZ + MODEL_MAXZ || z < m_mazeFloorZ - MODEL_MAXZ) {
        // if we are outside of the range the model is supposed to be in for Z, then
        // assume the ball is in the "hole".  Return the special value: m_mazeFloorZ - MODEL_MAXZ
        // to indicate this.
        z = m_mazeFloorZ - MODEL_MAXZ;
    }

    return z;
}

glm::vec3 FixedMaze::getRawNormalAtPosition(float x, float y) {
    return getRawNormalAtPosition(getXCell(x), getYCell(y));
}

glm::vec3 FixedMaze::getRawNormalAtPosition(size_t xcell, size_t ycell) {
    return m_normalMap[ycell * m_rowWidth +  xcell];
}

glm::vec3 FixedMaze::getNormalAtPosition(float x, float y) {
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
    return getNormalAtPosition(x, y, ballRadius(), velocity);
}

glm::vec3 FixedMaze::getNormalAtPosition(float x, float y, glm::vec3 const &velocity) {
    return getNormalAtPosition(x, y, ballRadius(), velocity);
}

glm::vec3 FixedMaze::getNormalAtPosition(float x, float y, float extend, glm::vec3 const &velocity) {
    glm::vec3 leastZNormal{0.0f, 0.0f, 1.0f};
    float leastDot = 0.0f;
    size_t xcellmin = getXCell(x - extend);
    size_t xcellmax = getXCell(x + extend);
    size_t ycellmin = getYCell(y - extend);
    size_t ycellmax = getYCell(y + extend);

    float speed = glm::length(velocity);
    for (size_t i = xcellmin; i <= xcellmax; i++) {
        for (size_t j = ycellmin; j <= ycellmax; j++) {
            glm::vec3 normal = getRawNormalAtPosition(i, j);
            if (std::fabs(normal.x) > m_floatErrorAmount || std::fabs(normal.y) > m_floatErrorAmount) {
                if (speed > m_floatErrorAmount) {
                    glm::vec2 velocityXY{velocity.x, velocity.y};
                    glm::vec2 normalXY = glm::normalize(glm::vec2{normal.x, normal.y});
                    float dot = glm::dot(velocityXY, normalXY);
                    if (leastDot > dot) {  // find the most negative dot product
                        leastZNormal = normal;
                        leastDot = dot;
                    }
                } else {
                    leastZNormal = normal;
                }
            }
        }
    }

    return leastZNormal;
}

glm::vec3 FixedMaze::getParallelAcceleration() {
    glm::vec3 normal = getRawNormalAtPosition(m_ball.position.x, m_ball.position.y);
    glm::vec3 normalGravityComponent = glm::dot(normal, m_ball.acceleration) * normal;

    return m_ball.acceleration - normalGravityComponent;
}

void FixedMaze::moveBall(float timeDiff) {
    checkBallBorders(m_ball.position, m_ball.velocity);
    glm::vec3 position = m_ball.position;
    glm::vec3 prevPosition = position;
    glm::vec3 velocity = getUpdatedVelocity(getParallelAcceleration(), timeDiff);

    auto adjustVelocity = [&] (bool isX, float direction) -> void {
        float speed = glm::length(velocity);
        if (m_extraBounce > 1.0f) {
            if (speed < m_minSpeedOnObjBounce) {
                if (speed > m_lengthTooSmallToNormalize) {
                    velocity = m_minSpeedOnObjBounce * glm::normalize(velocity);
                } else {
                    if (isX) {
                        velocity.x = direction * m_minSpeedOnObjBounce;
                    } else {
                        velocity.y = direction * m_minSpeedOnObjBounce;
                    }
                }
            }

            // Add in random velocity component perpendicular to the direction it is going in so
            // that we can't get stuck in loops where it bounces back and forth forever.
            speed = glm::length(velocity);
            if (speed > m_lengthTooSmallToNormalize) {
                glm::vec3 velocityXY{velocity.x, velocity.y, 0.0f};
                glm::vec3 perpendicularToVelocity = glm::cross(velocityXY, glm::vec3{0.0f, 0.0f, 1.0f});
                if (glm::length(perpendicularToVelocity) > m_lengthTooSmallToNormalize) {
                    perpendicularToVelocity = glm::normalize(perpendicularToVelocity);
                    float randomSpeed = m_randomNbrs.getFloat(-speed/5.0f, speed/5.0f);
                    velocity += randomSpeed * perpendicularToVelocity;
                }
            }
        }
    };
    uint32_t nbrComputations = 0;
    float lengthTraveled = 0;
    while (timeDiff > 0.0f && nbrComputations < 200) {
        nbrComputations ++;
        glm::vec3 surfaceNormal = getRawNormalAtPosition(position.x, position.y);
        velocity -= glm::dot(velocity, surfaceNormal) * surfaceNormal;
        float speed = glm::length(velocity);
        if (speed < m_floatErrorAmount) {
            // velocity is too small.  Return.
            break;
        }

        if (speed > m_speedLimit) {
            velocity = m_speedLimit * glm::normalize(velocity);
        }

        lengthTraveled += glm::length(prevPosition - position);
        if (lengthTraveled > ballDiameter()) {
            // the ball went too far.  Let the drawer draw it before moving any further.
            break;
        }

        glm::vec3 nextPos = position + velocity * timeDiff;
        size_t xcell = getXCell(position.x);
        size_t ycell = getYCell(position.y);

        float deltax = nextPos.x - position.x;
        float deltay = nextPos.y - position.y;
        std::array<float, 4> fractionsTillWall =  { -1.0f, -1.0f, -1.0f, -1.0f };

        if (deltax > m_floatErrorAmount || deltax < -m_floatErrorAmount) {
            // left
            fractionsTillWall[0] = (xcell * m_width / m_rowWidth - m_width / 2.0f - position.x) / deltax;

            // right
            fractionsTillWall[1] = ((xcell + 1) * m_width / m_rowWidth - m_width/2.0f - position.x) / deltax;
        }

        if (deltay > m_floatErrorAmount ||  deltay < -m_floatErrorAmount) {
            // down
            fractionsTillWall[2] = (ycell * m_height / m_rowHeight - m_height / 2.0f - position.y) / deltay;

            // up
            fractionsTillWall[3] = ((ycell + 1) * m_height / m_rowHeight - m_height / 2.0f - position.y) / deltay;
        }

        size_t smallest = fractionsTillWall.size();
        for (size_t i = 0; i < fractionsTillWall.size(); i++) {
            if (fractionsTillWall[i] < m_floatErrorAmount) {
                continue;
            }

            if (smallest >= fractionsTillWall.size()) {
                smallest = i;
            } else if (fractionsTillWall[i] < fractionsTillWall[smallest]) {
                smallest = i;
            }
        }

        if (smallest >= fractionsTillWall.size() || fractionsTillWall[smallest] >= 1.0f) {
            // moving within a cell - just move and break.
            nextPos.z = getZPos(nextPos.x, nextPos.y) + ballRadius();
            if (nextPos.z > position.z + ballDiameter()) {
                if (m_bounce) {
                    if (std::fabs(nextPos.x - position.x) > std::fabs(nextPos.y - position.y)) {
                        velocity.x = -m_extraBounce * velocity.x;
                        adjustVelocity(true, glm::sign(velocity.x));
                    } else {
                        velocity.y = -m_extraBounce * velocity.y;
                        adjustVelocity(false, glm::sign(velocity.y));
                    }
                } else {
                    velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                }
                break;
            }
            prevPosition = position;
            position = nextPos;
            break;
        }

        glm::vec3 newPos = position;
        switch (smallest) {
            case 0:
                // move to the cell to the left of us.
                newPos.x += (nextPos.x - position.x) * fractionsTillWall[smallest] - m_floatErrorAmount;
                newPos.y += (nextPos.y - position.y) * fractionsTillWall[smallest];
                break;
            case 1:
                // move to the cell to the right of us.
                newPos.x += (nextPos.x - position.x) * fractionsTillWall[smallest] + m_floatErrorAmount;
                newPos.y += (nextPos.y - position.y) * fractionsTillWall[smallest];
                break;
            case 2:
                // move to the cell below us
                newPos.y += (nextPos.y - position.y) * fractionsTillWall[smallest] - m_floatErrorAmount;
                newPos.x += (nextPos.x - position.x) * fractionsTillWall[smallest];
                break;
            case 3:
                // move to the cell above us
                newPos.y += (nextPos.y - position.y) * fractionsTillWall[smallest] + m_floatErrorAmount ;
                newPos.x += (nextPos.x - position.x) * fractionsTillWall[smallest];
                break;
            default:
                // shouldn't happen
                return;
        }


        if (checkBallBorders(newPos, velocity)) {
            prevPosition = position;
            timeDiff -= fractionsTillWall[smallest] * timeDiff;
            position = newPos;
            position.z = getZPos(position.x, position.y) + ballRadius();
            if (m_bounce) {
                continue;
            } else {
                break;
            }
        }

        glm::vec2 newPosXY{newPos.x, newPos.y};
        glm::vec2 positionXY{position.x, position.y};
        glm::vec2 nextPosXY{nextPos.x, nextPos.y};
        float timeInc = glm::length(newPosXY - positionXY) / glm::length(nextPosXY - positionXY) * timeDiff;

        newPos.z = getZPos(newPos.x, newPos.y) + ballRadius();
        if (newPos.z > position.z + ballDiameter()) {
            if (m_bounce) {
                glm::vec3 normal = getNormalAtPosition(newPos.x, newPos.y, velocity);
                auto doBounce = [&]() -> void {
                    size_t xnewcell = getXCell(newPos.x);
                    size_t ynewcell = getYCell(newPos.y);
                    if (xnewcell != xcell) {
                        velocity.x = -m_extraBounce * velocity.x;
                        adjustVelocity(true, glm::sign(static_cast<float>(xnewcell) - xcell));
                    }
                    if (ynewcell != ycell) {
                        velocity.y = -m_extraBounce * velocity.y;
                        adjustVelocity(false, glm::sign(static_cast<float>(ynewcell) - ycell));
                    }
                    timeDiff -= timeInc;
                };
                if (std::fabs(normal.x) < m_floatErrorAmount && std::fabs(normal.y) < m_floatErrorAmount) {
                    /* x and y components of the normal too small to compute the reflective
                     * velocity.  Just negate x and y velocity components so it goes back in the
                     * direction it came from.
                     */
                    doBounce();
                    continue;
                }
                glm::vec3 xyNormal = glm::normalize(glm::vec3{normal.x, normal.y, 0.0f});
                float velocityNormalToSurface = glm::dot(xyNormal, velocity);
                if (velocityNormalToSurface < 0.0f) {
                    velocity = velocity - 2.0f * m_extraBounce * velocityNormalToSurface * xyNormal;
                    adjustVelocity(velocity.x > velocity.y, glm::sign(glm::max(velocity.x, velocity.y)));
                    timeDiff -= timeInc;
                    continue;
                } else {
                    doBounce();
                    continue;
                }
            } else {
                velocity = glm::vec3{0.0f, 0.0f, 0.0f};
                notValid(velocity);
                timeDiff -= timeInc;
                break;
            }
        }
        prevPosition = position;
        position = newPos;
        timeDiff -= timeInc;
        if (position.z <= m_mazeFloorZ - MODEL_MAXZ + ballRadius() + m_floatErrorAmount) {
            m_finished = true;
            break;
        }
    }

    //surfaceNormal = getRawNormalAtPosition(position.x, position.y);
    //velocity -= glm::dot(velocity, surfaceNormal) * surfaceNormal;
    notValid(velocity);

    m_ball.velocity = velocity;
    m_ball.position = position;
}

bool FixedMaze::updateData() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    moveBall(timeDiff);

    updateRotation(timeDiff);

    return drawingNecessary();
}

bool FixedMaze::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (objs.empty() && textures.empty()) {
        /* floor */
        objs.emplace_back(m_floor, nullptr);
        textures.insert(std::make_pair(m_floor->texture, nullptr));
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
        /* ball */
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

FixedMaze::FixedMaze(std::shared_ptr<GameRequester> inGameRequester,
    std::shared_ptr<FixedMazeSaveData>,
    float width, float height, float mazeFloorZ)
    : Level{inGameRequester, width, height, mazeFloorZ, false, 1.0f/50.0f},
    m_extraBounce{1.0f},
    m_minSpeedOnObjBounce{0.0f},
    m_speedLimit{m_diagonal/4.0f}
{
}

void FixedMaze::findModelViewPort(
    std::vector<float> const &depthMap,
    std::vector<glm::vec3> const &normalMap,
    size_t rowWidth,
    glm::mat4 &trans,
    std::vector<float> &outDepthMap,
    std::vector<glm::vec3> &outNormalMap,
    size_t &outRowWidth)
{
    size_t i = 0;
    for (; i < depthMap.size(); i++) {
        if (depthMap[i] < m_mazeFloorZ - MODEL_MAXZ + m_floatErrorAmount) {
            break;
        }
    }

    if (i >= depthMap.size()) {
        // shouldn't happen
        throw std::runtime_error("Model finish condition not found.");
    }

    size_t column = i % rowWidth;
    size_t row = i / rowWidth;

    size_t viewPortWidth = static_cast<size_t>(std::ceil(rowWidth/m_height*m_width));
    if (viewPortWidth > rowWidth) {
        viewPortWidth = rowWidth;
    }

    size_t beginColumn;
    size_t mapBallWidth = static_cast<size_t>(std::floor(m_scaleBall * m_originalBallDiameter/2.0f/m_width *viewPortWidth));
    if (column > rowWidth - viewPortWidth) {
        beginColumn = rowWidth - viewPortWidth;
    } else if (column < viewPortWidth) {
        beginColumn = 0;
    } else {
        beginColumn = column - viewPortWidth / 2;
    }

    float x = (static_cast<int32_t>(rowWidth/2) -
            static_cast<int32_t>(viewPortWidth/2) - static_cast<int32_t>(beginColumn))*m_width/viewPortWidth;
    trans = glm::translate(glm::mat4(1.0f), glm::vec3{x, 0.0f, m_mazeFloorZ});

    // rowWidth is the same as rowHeight
    outDepthMap.resize(viewPortWidth * rowWidth);
    outNormalMap.resize(viewPortWidth * rowWidth);
    for (size_t j = beginColumn; j < beginColumn + viewPortWidth; j++) {
        for (size_t k = 0; k < rowWidth; k ++) {
            outDepthMap[k * viewPortWidth + j - beginColumn] = depthMap[k * rowWidth + j];
            outNormalMap[k * viewPortWidth + j - beginColumn] = normalMap[k * rowWidth + j];
        }
    }

    outRowWidth = viewPortWidth;
}

void FixedMaze::init()
{
    m_prevTime = std::chrono::high_resolution_clock::now();

    loadModel(m_gameRequester->getAssetStream(m_ballModel), m_ballVertices, m_ballIndices);

    m_floor = std::make_shared<DrawObject>();
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> verticesWithVertexNormals;
    loadModel(m_gameRequester->getAssetStream(m_floorModel), m_floor->vertices,
            m_floor->indices, &verticesWithVertexNormals);

    // For getting the depth map and normal map
    auto worldObj = std::make_shared<DrawObject>();
    worldObj->vertices = verticesWithVertexNormals.first;
    worldObj->indices = verticesWithVertexNormals.second;
    worldObj->modelMatrices.push_back(
            glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, m_mazeFloorZ}) *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_height/m_modelSize, m_height/m_modelSize, 1.0f}) *
            glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
    worldObj->texture = nullptr;
    DrawObjectTable worldMap;
    worldMap.emplace_back(worldObj, nullptr);

    std::vector<float> depthMap;
    std::vector<glm::vec3> normalMap;
    m_rowHeight = static_cast<uint32_t>(std::floor(10.0f/m_scaleBall * m_width));

    // The model is a square.  Get the whole depth and normal maps (without stretching.  Then
    // find the hole and cut the model so that the hole appears in the center if possible, otherwise
    // on the side.  So we pass in m_height for the width and height.
    m_gameRequester->getDepthTexture(worldMap, m_height, m_height,
            m_rowHeight /* height is the same as width */,
            m_mazeFloorZ - MODEL_MAXZ, m_mazeFloorZ + MODEL_MAXZ,
            depthMap, normalMap);

    // cut the model so that the hole appears in the center if possible.
    glm::mat4 trans;
    findModelViewPort(depthMap, normalMap, m_rowHeight, trans, m_depthMap, m_normalMap, m_rowWidth);

    m_floor->modelMatrices.push_back(
            trans *
            glm::scale(glm::mat4(1.0f), glm::vec3{m_height/m_modelSize, m_height/m_modelSize, 1.0f}) *
            glm::mat4_cast(glm::angleAxis(3.1415926f/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
    m_floor->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_floorTexture);

    // search for a starting point in a row on the model floor (not a high up point).
    float x = -m_width / 2.0f + ballRadius();
    do {
        m_ball.position = {x, -m_height / 2.0f + ballRadius(), 0.0f};
        m_ball.position.z = getZPos(m_ball.position.x, m_ball.position.y);
        x += m_width/m_rowWidth;
        if (x > m_width / 2.0f - ballRadius()) {
            // give up...
            m_ball.position.x = -m_width / 2.0f + ballRadius();
            break;
        }
    } while (m_ball.position.z > m_mazeFloorZ + m_floatErrorAmount || m_ball.position.z < m_mazeFloorZ - m_floatErrorAmount);
    m_ball.position.z += ballRadius();
}
