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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../common.hpp"
#include "../../saveData.hpp"
#include "../level.hpp"

struct MovablePassageSaveData : public LevelSaveData {
    static int constexpr m_movablePassageVersion = 1;
    MovablePassageSaveData() : LevelSaveData{m_movablePassageVersion} {}
};

class Component {
public:
    enum MovableType {
        noneAssigned = 0,
        staticObj,
        dynamicObj
    };

    enum CellWall {
        wallRight = 0,
        wallUp,
        wallLeft,
        wallDown,
        wallMax = 3,
        noWall
    };

    enum ComponentType {
        straight = 0,
        tjunction,
        crossjunction,
        turn,
        open,
        closedBottom,
        closedCorner,
        maxComponentAllowingBall = 6,
        noMovementDirt,
        noMovementRock,
        maxComponentType = 8
    };

    using checkForNextWallFunc = std::function<std::pair<bool, bool>(Component::CellWall, Component::CellWall)>;
    std::pair<CellWall, CellWall> moveBallInCell(
            size_t placementIndex,
            glm::vec3 &position,
            float &timediff,
            glm::vec3 &velocity,
            float ballRadius,
            checkForNextWallFunc checkForNextWall)
    {
        // unrotate the position and velocity then move the ball in the cell, then re-rotate them
        glm::mat4 rot = glm::rotate(glm::mat4{1.0f}, -m_placements[placementIndex].rotationAngle(),
                glm::vec3{0.0f, 0.0f, 1.0f});
        glm::vec4 rpos4 = rot * glm::vec4{position.x, position.y, 0.0f, 0.0f};
        glm::vec4 rvel4 = rot * glm::vec4{velocity.x, velocity.y, 0.0f, 0.0f};
        glm::vec3 rpos = glm::vec3{rpos4.x, rpos4.y, 0.0f};
        glm::vec3 rvel = glm::vec3{rvel4.x, rvel4.y, 0.0f};
        auto ret = moveBallInCellFuncs[m_componentType](rpos, timediff, rvel, ballRadius, checkForNextWall);
        rot = glm::rotate(glm::mat4{1.0f}, m_placements[placementIndex].rotationAngle(),
                glm::vec3{0.0f, 0.0f, 1.0f});
        rpos4 = rot * glm::vec4{rpos.x, rpos.y, 0.0f, 0.0f};
        rvel4 = rot * glm::vec4{rvel.x, rvel.y, 0.0f, 0.0f};
        position = glm::vec3{rpos4.x, rpos4.y, 0.0f};
        velocity = glm::vec3{rvel4.x, rvel4.y, 0.0f};
        if (ret.first != CellWall::noWall) {
            uint32_t nbr90degreeRotations = static_cast<uint32_t>(
                    std::floor(m_placements[placementIndex].rotationAngle()/glm::radians(90.0f)));
            ret.first = static_cast<CellWall>((ret.first + nbr90degreeRotations) % (CellWall::wallMax+1));
        }
        if (ret.second != CellWall::noWall) {
            uint32_t nbr90degreeRotations = static_cast<uint32_t>(
                    std::floor(m_placements[placementIndex].rotationAngle()/glm::radians(90.0f)));
            ret.second = static_cast<CellWall>((ret.second + nbr90degreeRotations) % (CellWall::wallMax+1));
        }
        return ret;
    }

    std::pair<CellWall, CellWall> moveBallInJunction(
        glm::vec3 &position,
        float &timediff,
        glm::vec3 &velocity,
        float ballRadius,
        checkForNextWallFunc checkForNextWall)
    {
        auto checkBallBorder = [&](float &p, float &speed, CellWall wall, float border, int32_t sign)
                -> std::pair<CellWall, CellWall>
        {
            auto ret = checkForNextWall(wall, CellWall::noWall);
            if (ret.first) {
                p = border + sign * Level::m_floatErrorAmount;
                return std::make_pair(wall, CellWall::noWall);
            } else {
                p = border - sign * ballRadius;
                speed = 0.0f;
                return std::make_pair(CellWall::noWall, CellWall::noWall);
            }
        };

        auto checkBallBorder2 = [&](float &p, float &speed, CellWall wall, float border, int32_t sign)
                -> std::pair<CellWall, CellWall>
        {
            auto ret = checkForNextWall(wall, CellWall::noWall);
            if (!ret.first) {
                p = border - sign * ballRadius;
                speed = 0.0f;
            }
            return std::make_pair(CellWall::noWall, CellWall::noWall);
        };

        auto moveBallCorridor = [&](float &p, float &speed, CellWall wall1, float border1, CellWall wall2, float border2)
                -> std::pair<CellWall, CellWall>
        {
            if (p < border1) {
                return checkBallBorder(p, speed, wall1, border1, -1);
            } else if (p < border1 + ballRadius) {
                return checkBallBorder2(p, speed, wall1, border1, -1);
            } else if (p > border2) {
                return checkBallBorder(p, speed, wall2, border2, 1);
            } else if (p > border2 - ballRadius) {
                return checkBallBorder2(p, speed, wall2, border2, 1);
            }
            return std::make_pair(CellWall::noWall, CellWall::noWall);
        };

        if (velocity.x == 0.0f && velocity.y == 0.0f) {
            return std::make_pair(CellWall::noWall, CellWall::noWall);
        }

        glm::vec3 nextPos = position + velocity * timediff;

        if (fabs(nextPos.x) > fabs(nextPos.y)) {
            nextPos.y = 0.0f;
            auto ret = moveBallCorridor(nextPos.x, velocity.x,
                                        CellWall::wallLeft, -m_componentSize/2,
                                        CellWall::wallRight, m_componentSize/2);
            position = nextPos;
            return ret;
        } else {
            nextPos.x = 0.0f;
            auto ret = moveBallCorridor(nextPos.y, velocity.y,
                                        CellWall::wallDown, -m_componentSize/2,
                                        CellWall::wallUp, m_componentSize/2);
            position = nextPos;
            return ret;
        }
    }

    std::pair<CellWall, CellWall> moveBallInOpenArea(
            float leftWall,
            bool isWallLeft,
            float rightWall,
            bool isWallRight,
            float bottomWall,
            bool isWallBottom,
            float topWall,
            bool isWallTop,
            glm::vec3 &position,
            float &timediff,
            glm::vec3 &velocity,
            float ballRadius,
            checkForNextWallFunc checkForNextWall)
    {
        std::array<float, 4> differences = {
                fabs(velocity.x) < Level::m_floatErrorAmount ? -1 : (rightWall - position.x)/velocity.x,
                fabs(velocity.x) < Level::m_floatErrorAmount ? -1 : (leftWall - position.x)/velocity.x,
                fabs(velocity.y) < Level::m_floatErrorAmount ? -1 : (topWall - position.y)/velocity.y,
                fabs(velocity.y) < Level::m_floatErrorAmount ? -1 : (bottomWall - position.y)/velocity.y
        };

        size_t largestDifferenceIndex = differences.size();
        for (size_t i = 0; i < differences.size(); i++) {
            if (differences[i] < Level::m_floatErrorAmount) {
                continue;
            }
            if (largestDifferenceIndex >= differences.size()) {
                largestDifferenceIndex = i;
                continue;
            }
            if (differences[i] < differences[largestDifferenceIndex])
            {
                largestDifferenceIndex = i;
            }
        }

        CellWall wall1 = CellWall::noWall;
        float timeDiffTillEdge = 0;
        if (largestDifferenceIndex >= differences.size() ||
            differences[largestDifferenceIndex] > timediff)
        {
            // movement within cell
            timeDiffTillEdge = timediff;
        } else {
            timeDiffTillEdge = differences[largestDifferenceIndex];
            if (largestDifferenceIndex == 0) {
                wall1 = CellWall::wallRight;
            } else if (largestDifferenceIndex == 1) {
                wall1 = CellWall::wallLeft;
            } else if (largestDifferenceIndex == 2) {
                wall1 = CellWall::wallUp;
            } else if (largestDifferenceIndex == 3) {
                wall1 = CellWall::wallDown;
            }
        }

        glm::vec3 nextPos = position + velocity * timeDiffTillEdge;
        timediff -= timeDiffTillEdge;

        CellWall wall2 = CellWall::noWall;
        if (wall1 == CellWall::noWall) {
            if (nextPos.y > topWall) {
                wall1 = CellWall::wallUp;
            } else if (nextPos.y < bottomWall) {
                wall1 = CellWall::wallDown;
            }
            if (nextPos.x > rightWall) {
                wall2 = CellWall::wallRight;
            } else if (nextPos.x < leftWall) {
                wall2 = CellWall::wallLeft;
            }
        } else if (wall1 == CellWall::wallLeft || wall1 == CellWall::wallRight) {
            if (nextPos.y > topWall) {
                wall2 = CellWall::wallUp;
            } else if (nextPos.y < bottomWall) {
                wall2 = CellWall::wallDown;
            }
        } else {
            if (nextPos.x > rightWall) {
                wall2 = CellWall::wallRight;
            } else if (nextPos.x < leftWall) {
                wall2 = CellWall::wallLeft;
            }
        }

        // check for hitting the end point but that end point is not a wall.  Allow
        // advancement all the way to that wall.
        auto checkCellWall = [&] (bool isWall, CellWall conditionalWall, float &pos, float &vel, float wallPos) -> void {
            if (!isWall && wall1 == conditionalWall) {
                wall1 = CellWall::noWall;
                pos = wallPos;
                vel = 0.0f;
            } else if (!isWall && wall2 == conditionalWall) {
                wall2 = CellWall::noWall;
                pos = wallPos;
                vel = 0.0f;
            }
        };
        checkCellWall(isWallLeft, CellWall::wallLeft, nextPos.x, velocity.x, leftWall);
        checkCellWall(isWallRight, CellWall::wallRight, nextPos.x, velocity.x, rightWall);
        checkCellWall(isWallTop, CellWall::wallUp, nextPos.y, velocity.y, topWall);
        checkCellWall(isWallBottom, CellWall::wallDown, nextPos.y, velocity.y, bottomWall);

        if (wall1 == CellWall::noWall && wall2 == CellWall::noWall) {
            position = nextPos;
            return std::make_pair(wall1, wall2);
        }

        // check for hitting an actual wall.  Allow advancement into the next cell if there is no
        // wall there.
        auto ret = checkForNextWall(wall1, wall2);
        auto checkCellWall2 = [&] (CellWall wall, float wallPos, float &pos, int32_t sign) {
            if (((ret.first && wall1 == wall) || (ret.second && wall2 == wall))) {
                pos = wallPos + sign * Level::m_floatErrorAmount;
            } else if ((!ret.first && wall1 == wall) || (!ret.second && wall2 == wall)) {
                pos = wallPos - sign * ballRadius;
            }
        };
        checkCellWall2(CellWall::wallLeft, leftWall, nextPos.x, -1);
        checkCellWall2(CellWall::wallRight, rightWall, nextPos.x, 1);
        checkCellWall2(CellWall::wallDown, bottomWall, nextPos.y, -1);
        checkCellWall2(CellWall::wallUp, topWall, nextPos.y, 1);

        position = nextPos;
        if (!ret.first) {
            wall1 = CellWall::noWall;
        }
        if (!ret.second) {
            wall2 = CellWall::noWall;
        }
        return std::make_pair(wall1, wall2);
    }

    // return a std::pair of CellWall indicating which walls were hit (and passed through).  If
    // no walls were hit, then return <noWall, noWall>.
    using MoveBallInCellFunc = std::function<std::pair<CellWall, CellWall>(glm::vec3 &, float &, glm::vec3 &, float, checkForNextWallFunc)>;
    std::array<MoveBallInCellFunc, ComponentType::maxComponentAllowingBall + 1> const moveBallInCellFuncs = {
        // straight
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                position.x = 0.0f;
                velocity.x = 0.0f;

                if (velocity.y == 0.0f) {
                    return std::make_pair(CellWall::noWall, CellWall::noWall);
                }

                return moveBallInJunction(position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // T-Junction
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                glm::vec3 nextPos = position + velocity * timediff;
                if (nextPos.y > 0.0f || position.y > 0.0f) {
                    position.y = 0.0f;
                    velocity.y = 0.0f;
                }

                return moveBallInJunction(position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // cross junction
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                return moveBallInJunction(position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // turn
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                glm::vec3 nextPos = position + velocity * timediff;
                if (nextPos.y > 0.0f || position.y > 0.0f) {
                    position.y = 0.0f;
                    velocity.y = 0.0f;
                }
                if (nextPos.x > 0.0f || position.x > 0.0f) {
                    position.x = 0.0f;
                    velocity.x = 0.0f;
                }

                return moveBallInJunction(position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // open area
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall> {
                return moveBallInOpenArea(-m_componentSize/2, true, m_componentSize/2, true,
                                          -m_componentSize/2, true, m_componentSize/2, true,
                                          position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // closed bottom
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                return moveBallInOpenArea(-m_componentSize/2, true, m_componentSize/2, true,
                                          0.0f, false, m_componentSize/2, true,
                                          position, timediff, velocity, ballRadius, checkForNextWall);
            }),

        // closed corner (left and bottom walls closed)
        MoveBallInCellFunc(
            [&](glm::vec3 &position,
                    float &timediff,
                    glm::vec3 &velocity,
                    float ballRadius,
                    checkForNextWallFunc checkForNextWall) -> std::pair<CellWall, CellWall>
            {
                return moveBallInOpenArea(0.0f, false, m_componentSize/2, true,
                                          0.0f, false, m_componentSize/2, true,
                                          position, timediff, velocity, ballRadius, checkForNextWall);
            })
    };

    class Placement {
    public:
        static uint32_t constexpr m_invalidObjReference = std::numeric_limits<uint32_t>::max();

        /* accessors */
        uint32_t row() { return m_row; }
        uint32_t col() { return m_col; }
        float rotationAngle () { return m_rotationAngle; }
        bool lockedIntoPlace() { return m_lockedIntoPlace; }
        auto &prev() { return m_prev; }
        auto &next() { return m_next; }
        glm::vec2 movePositionSoFar() { return m_movePositionSoFar; }
        uint32_t dynObjReference() { return m_dynObjReference; }

        /* setters */
        void setRC(uint32_t row, uint32_t col) { m_row = row; m_col = col; }
        void setDynObjReference(uint32_t ref) { m_dynObjReference = ref; }
        void rotate() {
            m_rotationAngle += glm::radians(90.0f);
            float twopi = 2 * glm::radians(180.0f);
            while (m_rotationAngle > twopi) {
                m_rotationAngle -= twopi;
            }
        }

        void movePlacement(glm::vec2 const &fractionalMove) {
            if (m_moveInProgress) {
                m_movePositionSoFar += fractionalMove;
            } else {
                m_moveInProgress = true;
                m_movePositionSoFar = fractionalMove;
            }
        }
        void moveDone() {
            m_moveInProgress = false;
        }

        Placement(uint32_t row = 0,
                  uint32_t col = 0,
                  float rotationAngle = 0.0f, /* radians */
                  bool lockedIntoPlace = false,
                  std::shared_ptr<Component> prevComponent = nullptr,
                  size_t prevIndex = 0,
                  std::shared_ptr<Component> nextComponent = nullptr,
                  size_t nextIndex = 0)
                : m_row{row},
                  m_col{col},
                  m_rotationAngle{rotationAngle},
                  m_lockedIntoPlace{lockedIntoPlace},
                  m_prev{std::make_pair(prevComponent, prevIndex)},
                  m_next{std::make_pair(nextComponent, nextIndex)},
                  m_moveInProgress{false},
                  m_movePositionSoFar{0.0f, 0.0f},
                  m_dynObjReference{m_invalidObjReference}
        {
        }
    private:
        uint32_t m_row;
        uint32_t m_col;
        float m_rotationAngle; /*radians */
        bool m_lockedIntoPlace;
        std::pair<std::shared_ptr<Component>, size_t> m_prev;
        std::pair<std::shared_ptr<Component>, size_t> m_next;
        bool m_moveInProgress;
        glm::vec2 m_movePositionSoFar;
        uint32_t m_dynObjReference;
    };

    uint32_t add(
            uint32_t row = 0,
            uint32_t col = 0,
            float rotationAngle = 0.0f, /* radians */
            bool lockedIntoPlace = false,
            std::shared_ptr<Component> prevComponent = nullptr,
            size_t prevIndex = 0,
            std::shared_ptr<Component> nextComponent = nullptr,
            size_t nextIndex = 0)
    {
        m_placements.emplace_back(row, col, rotationAngle, lockedIntoPlace,
                prevComponent, prevIndex, nextComponent, nextIndex);
        return m_placements.size() - 1;
    }

    bool hasWallAt(CellWall wall, size_t placementNumber) {
        // rotate the exitPoint that we are checking backwards by the amount the component is
        // rotated forwards so that we only have to do this once.  Rotating backwards is the same
        // as going around a whole turn minus the angle the component is rotated by.
        auto nbr90degreeRotations = 4-static_cast<uint32_t>(
                std::floor(m_placements[placementNumber].rotationAngle()/glm::radians(90.0f)));
        wall = static_cast<CellWall>((wall + nbr90degreeRotations) % (CellWall::wallMax+1));
        return m_cellWalls.count(wall) > 0;
    }

    CellWall actualWall(CellWall wall, size_t placementNumber) {
        if (wall == CellWall::noWall) {
            return wall;
        }
        auto nbr90degreeRotations = static_cast<uint32_t>(
                std::floor(m_placements[placementNumber].rotationAngle()/glm::radians(90.0f)));
        return static_cast<CellWall>((wall + nbr90degreeRotations) % (CellWall::wallMax+1));
    }

    bool operator==(Component const &other) { return other.m_componentType == m_componentType; }
    bool operator<(Component const &other) { return m_componentType < other.m_componentType; }
    auto placementsBegin() { return m_placements.begin(); }
    auto placementsEnd() { return m_placements.end(); }
    auto type() { return m_componentType; }
    Placement &placement(size_t index) { return m_placements[index]; }
    size_t nbrPlacements() { return m_placements.size(); }
    float componentSize() { return m_componentSize; }
    std::vector<size_t> dynObjReferences() { return m_dynObjReferences; }
    std::vector<size_t> dynObjReferencesLockedComponent() { return m_dynObjReferencesLockedComponent; }

    void setSize(float tileSize) { m_componentSize = tileSize; }
    void setDynObjReferences(std::vector<size_t> refs) { m_dynObjReferences = refs; }
    void setDynObjReferencesLockedComponent(std::vector<size_t> refs) { m_dynObjReferencesLockedComponent = refs; }

    Component(ComponentType inType, float componentSize = 0.0f)
        : m_componentType{inType},
        m_componentSize{componentSize}
    {
        switch (m_componentType) {
        case ComponentType::tjunction:
            m_cellWalls.insert(CellWall::wallUp);
            break;
        case ComponentType::straight:
            m_cellWalls.insert(CellWall::wallLeft);
            m_cellWalls.insert(CellWall::wallRight);
            break;
        case ComponentType::turn:
            m_cellWalls.insert(CellWall::wallRight);
            m_cellWalls.insert(CellWall::wallUp);
            break;
        case ComponentType::closedBottom:
            m_cellWalls.insert(CellWall::wallDown);
            break;
        case ComponentType::closedCorner:
            m_cellWalls.insert(CellWall::wallDown);
            m_cellWalls.insert(CellWall::wallLeft);
            break;
        case ComponentType::noMovementDirt:
        case ComponentType::noMovementRock:
            m_cellWalls.insert(CellWall::wallRight);
            m_cellWalls.insert(CellWall::wallUp);
            m_cellWalls.insert(CellWall::wallLeft);
            m_cellWalls.insert(CellWall::wallDown);
            break;
        case ComponentType::open:
        case ComponentType::crossjunction:
        default:
            break;
        }
    }

private:
    ComponentType const m_componentType;
    float m_componentSize;
    std::vector<Placement> m_placements;
    std::set<CellWall> m_cellWalls;
    std::vector<size_t> m_dynObjReferences;
    std::vector<size_t> m_dynObjReferencesLockedComponent;
};

class GameBoardBlock {
public:
    enum BlockType {
        begin,
        onBoard,
        offBoard,
        endOffBoard,
        end
    };

    BlockType blockType() { return m_blockType; }
    std::shared_ptr<Component> const &component() { return m_component; }
    size_t placementIndex() { return m_placementIndex; }
    std::shared_ptr<Component> const &secondaryComponent() { return m_secondaryComponent; }
    size_t secondaryPlacementIndex() { return m_secondaryPlacementIndex; }

    void setBlockType(BlockType blockType) { m_blockType = blockType; }

    void setComponent(std::shared_ptr<Component> component, size_t placementIndex) {
        m_component = std::move(component);
        m_placementIndex = placementIndex;
    }

    void setSecondaryComponent(std::shared_ptr<Component> component, size_t placementIndex) {
        m_secondaryComponent = std::move(component);
        m_secondaryPlacementIndex = placementIndex;
    }

    GameBoardBlock(
        BlockType blockType = BlockType::offBoard,
        std::shared_ptr<Component> component = nullptr,
        size_t placementIndex = 0)
        : m_blockType{blockType},
        m_component{component},
        m_placementIndex{placementIndex}
    {
    }
private:
    BlockType m_blockType;
    std::shared_ptr<Component> m_component;
    size_t m_placementIndex;
    std::shared_ptr<Component> m_secondaryComponent;
    size_t m_secondaryPlacementIndex;
};

class GameBoard {
public:
    static uint32_t constexpr m_nbrTileRowsForStart = 3;
    static uint32_t constexpr m_nbrTileRowsForEnd = 2;

    uint32_t widthInTiles() { return m_blocks.empty() ? 0 : m_blocks[0].size(); }
    uint32_t heightInTiles() { return m_blocks.size(); }
    bool drag(glm::vec2 const &startPosition, glm::vec2 const &distance);
    bool dragEnded(glm::vec2 const &endPosition);
    bool tap(glm::vec2 const &position);
    std::pair<uint32_t, uint32_t> findRC(glm::vec2 postion);
    float blockSize() { return m_blockSize; }
    bool isMoveInProgress() { return m_moveInProgress; }
    std::pair<uint32_t, uint32_t> moveRC() { return m_moveStartingPosition; }
    float getZPosEndTile() { return m_centerPos.z - 3*m_blockSize/4.0f ; }
    void setCenterPos(glm::vec3 const &pos) {
        m_centerPos = pos;
    }
    std::pair<bool, bool> checkforNextWall(Component::CellWall wall1, Component::CellWall wall2,
                                           uint32_t ballRow, uint32_t ballCol);

    void initialize(float tileSize, glm::vec3 const &pos, uint32_t rows, uint32_t cols) {
        m_blockSize = tileSize;
        m_width = cols * tileSize;
        m_height = rows * tileSize;
        m_centerPos = pos;
        m_blocks.resize(rows);
        for (auto &block : m_blocks) {
            block.resize(cols);
        }
    }

    /* row 0 is on the bottom, col 0 is on the left */
    glm::vec3 position(uint32_t row, uint32_t col) {
        float x, y, z;
        x = m_blockSize * (col + 0.5f) - m_width / 2 + m_centerPos.x;
        if (m_blocks[row][col].blockType() == GameBoardBlock::BlockType::end ||
            m_blocks[row][col].blockType() == GameBoardBlock::BlockType::endOffBoard) {
            y = m_blockSize * (m_blocks.size() - m_nbrTileRowsForEnd/2.0f) - m_height/2 + m_centerPos.y;
            z = getZPosEndTile();
        } else {
            y = m_blockSize * (row + 0.5f) - m_height / 2 + m_centerPos.y;
            z = m_centerPos.z - m_blockSize / 2.0f;
        }
        return glm::vec3 { x, y, z };
    }

    glm::vec3 scaleEndObject() {
        return glm::vec3{m_blockSize, m_blockSize * m_nbrTileRowsForEnd, 1.0f};
    }

    GameBoardBlock &block(uint32_t row, uint32_t col) { return m_blocks[row][col]; }
    GameBoardBlock::BlockType blockType(uint32_t row, uint32_t col) { return m_blocks[row][col].blockType(); }

    GameBoard()
            : m_width{0.0f},
              m_height{0.0f},
              m_centerPos{0.0f, 0.0f, 0.0f}
    {}
private:
    float m_width;
    float m_height;

    // expected to be set to the x, y center of the game board and z is set to the where
    // the tops of the components should go.
    glm::vec3 m_centerPos;

    float m_blockSize;
    std::vector<std::vector<GameBoardBlock>> m_blocks;
    bool m_moveInProgress;
    std::pair<uint32_t, uint32_t> m_moveStartingPosition;

    bool hasMovableComponent(GameBoardBlock &b) {
        return (b.blockType() == GameBoardBlock::BlockType::offBoard ||
                b.blockType() == GameBoardBlock::BlockType::onBoard) &&
               b.component() != nullptr &&
               b.component()->type() != Component::ComponentType::noMovementRock &&
               b.component()->type() != Component::ComponentType::noMovementDirt;
    }
};

// All models expected to be exported with Z-up, Y-forward.
class MovablePassage : public Level {
public:
    bool drag(float startX, float startY, float distanceX, float distanceY) override;
    bool dragEnded(float x, float y) override;
    bool tap(float x, float y) override;
    glm::vec4 getBackgroundColor() override { return {0.6f, 0.8f, 1.0f, 1.0f}; };
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }
    void getLevelFinisherCenter(float &x, float &y) override { x = 0.0f; y = 0.0f; };
    SaveLevelDataFcn getSaveLevelDataFcn() override;

    // call after all other init functions are completed but before updateStaticDrawObjects
    std::vector<float> getAdditionalWHatZRequests() override {
        return std::vector<float>{m_gameBoard.getZPosEndTile()};
    }

    // call after getAdditionalWHatZRequests to return the requested width/height at z.
    void setAdditionalWH(float, float height, float z) override {
        if (z == m_gameBoard.getZPosEndTile()) {
            m_gameBoard.setCenterPos(glm::vec3{0.0f, (height - m_gameBoard.heightInTiles()* m_gameBoard.blockSize())/2.0f, m_mazeFloorZ});
        }
    }

    // this function can be called at any time before the level is started.
    void initSetBallInfo(
        std::string const &ballModel,
        std::string const &ballTexture)
    {
        m_ballModel = ballModel;
        m_ballTextureName = ballTexture;
    }

    // this function can be called at any time before the level is started.
    void initSetGameBoardInfo(
            std::string const &lockedComponentTexture,
            std::vector<std::string> const &blockedRockModel,
            std::string const &blockedRockTexture,
            std::vector<std::string> const &blockedDirtTexture,
            std::string const &componentTextureEnd,
            std::string const &textureEndOffBoard,
            std::string const &startCornerModel,
            std::string const &startCornerTexture,
            std::string const &startSideModel,
            std::vector<std::string> const &startSideTexture,
            std::string const &startOpenModel,
            std::vector<std::string> const &startOpenTexture)
    {
        m_textureLockedComponent = lockedComponentTexture;

        m_componentModels[Component::ComponentType::noMovementRock] = blockedRockModel;
        m_componentTextures[Component::ComponentType::noMovementRock].push_back(blockedRockTexture);
        m_componentTextures[Component::ComponentType::noMovementDirt] = blockedDirtTexture;

        m_componentTextureEnd = componentTextureEnd;
        m_textureEndOffBoard = textureEndOffBoard;

        m_componentModels[Component::ComponentType::closedCorner].push_back(startCornerModel);
        m_componentTextures[Component::ComponentType::closedCorner].push_back(startCornerTexture);

        m_componentModels[Component::ComponentType::closedBottom].push_back(startSideModel);
        m_componentTextures[Component::ComponentType::closedBottom] = startSideTexture;

        m_componentModels[Component::ComponentType::open].push_back(startOpenModel);
        m_componentTextures[Component::ComponentType::open] = startOpenTexture;
    }

    // all the compoents should be added before calling this function.
    void initSetGameBoard(
        uint32_t nbrTilesX,
        uint32_t nbrTilesY,
        uint32_t startCplumn,
        uint32_t endColumn);

    // this function can be called anytime before initSetGameBoard. This function must be called for
    // Component::ComponentType::straight even if the user is not allowed to use the straight
    // component.
    void initAddType(
        Component::ComponentType inComponentType,
        uint32_t nbrComponents,
        std::string const &model,
        std::string const &texture)
    {
        if (!texture.empty()) {
            m_componentTextures[inComponentType].push_back(texture);
        }
        if (!model.empty()) {
            m_componentModels[inComponentType].push_back(model);
        }
        for (uint32_t i = 0; i < nbrComponents; i++) {
            m_components[inComponentType]->add();
            m_nbrComponents++;
        }
    }

    // this function can be called at any time before initDone.
    void initAddRock(
        uint32_t row,
        uint32_t col)
    {
        m_addedRocks.emplace_back(row, col);
    }

    // all other init functions before calling this function or starting the level
    void initDone();

    ~MovablePassage() override = default;

    MovablePassage(
            std::shared_ptr<GameRequester> inGameRequester,
            std::shared_ptr<MovablePassageSaveData> /*sd*/,
            float width, float height, float maxZ)
            : Level(inGameRequester, width, height, maxZ, true, 1/50.0f, false),
              m_random{},
              m_zdrawTopsOfObjects{m_mazeFloorZ},
              m_zMovingPlacement{m_mazeFloorZ + m_scaleBall},
              m_components{
                      std::make_shared<Component>(Component::ComponentType::straight),
                      std::make_shared<Component>(Component::ComponentType::tjunction),
                      std::make_shared<Component>(Component::ComponentType::crossjunction),
                      std::make_shared<Component>(Component::ComponentType::turn),
                      std::make_shared<Component>(Component::ComponentType::open),
                      std::make_shared<Component>(Component::ComponentType::closedBottom),
                      std::make_shared<Component>(Component::ComponentType::closedCorner),
                      std::make_shared<Component>(Component::ComponentType::noMovementDirt),
                      std::make_shared<Component>(Component::ComponentType::noMovementRock)},
              m_gameBoard{},
              m_nbrComponents{0},
              m_texturesChanged{true},
              m_initDone{false},
              m_objsReferenceBall{0}
    {
    }

private:
    Random m_random;
    float const m_zMovingPlacement;
    float const m_zdrawTopsOfObjects;
    uint32_t m_ballRow;
    uint32_t m_ballCol;
    std::chrono::high_resolution_clock::time_point m_prevTime;

    std::array<std::shared_ptr<Component>, Component::ComponentType::maxComponentType + 1> m_components;

    GameBoard m_gameBoard;
    uint32_t m_nbrComponents;
    bool m_texturesChanged;
    bool m_initDone;

    std::string m_ballModel;
    std::string m_ballTextureName;
    uint32_t m_objsReferenceBall;

    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentModels;
    std::array<std::vector<std::string>, Component::ComponentType::maxComponentType + 1> m_componentTextures;
    std::string m_componentTextureEnd;
    std::string m_textureEndOffBoard;
    std::string m_textureLockedComponent;

    // rocks row and col do not consider the off game squares or the start or end.  Just
    // the middle of the game board.  (0,0) is at the bottom left part of the board.
    std::vector<std::pair<uint32_t, uint32_t>> m_addedRocks;

    // the row and column which starts the area in which users can place blocks
    std::pair<uint32_t, uint32_t> m_gameBoardStartRowColumn;

    // the row and column which ends the area in which users can place blocks
    std::pair<uint32_t, uint32_t> m_gameBoardEndRowColumn;

    // the column in which the end tile is located.
    uint32_t m_columnEndPosition;

    std::vector<size_t> addObjs(DrawObjectTable &objs, TextureMap &textures,
            std::vector<std::string> const &model, std::vector<std::string> const &textureNames);

    size_t chooseObj(std::shared_ptr<Component> const &component, size_t placementIndex);
    size_t addModelMatrixToObj(DrawObjectTable &objs, std::vector<size_t> const &refs,
                             std::shared_ptr<Component> const &component, size_t placementIndex,
                             glm::mat4 modelMatrix);
};
#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP */
