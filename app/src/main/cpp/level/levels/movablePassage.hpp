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

    CellWall moveBallInCell(size_t placementIndex, glm::vec3 &position, float &timediff, glm::vec3 &velocity) {
        glm::mat4 rot = glm::rotate(glm::mat4{1.0f}, m_placements[placementIndex].rotationAngle(),
                glm::vec3{0.0f, 0.0f, 1.0f});
        glm::vec4 rpos4 = rot * glm::vec4{position.x, position.y, position.z, 0.0f};
        glm::vec4 rvel4 = rot * glm::vec4{velocity.x, velocity.y, 0.0f, 0.0f};
        glm::vec3 rpos = glm::vec3{rpos4.x, rpos4.y, rpos4.z};
        glm::vec3 rvel = glm::vec3{rvel4.x, rvel4.y, rvel4.z};
        CellWall ret = moveBallInCellFuncs[m_componentType](rpos, timediff, rvel);
        rot = glm::rotate(glm::mat4{1.0f}, -m_placements[placementIndex].rotationAngle(),
                glm::vec3{0.0f, 0.0f, 1.0f});
        rpos4 = rot * glm::vec4{rpos.x, rpos.y, rpos.z, 0.0f};
        rvel4 = rot * glm::vec4{rvel.x, rvel.y, 0.0f, 0.0f};
        position = glm::vec3{rpos4.x, rpos4.y, rpos4.z};
        velocity = glm::vec3{rvel4.x, rvel4.y, 0.0f};
        if (ret != CellWall::noWall) {
            uint32_t nbr90degreeRotations = static_cast<uint32_t>(
                    std::floor(m_placements[placementIndex].rotationAngle()/glm::radians(90.0f)));
            ret = static_cast<CellWall>((ret + nbr90degreeRotations) % (CellWall::wallMax+1));
        }
        return ret;
    }

    // start < end
    // returns true if an end point is hit, false otherwise.
    bool moveBall(float &pos, float &timediff, float start, float end, float speed) {
        float posnext = pos + speed * timediff;
        if (posnext <= start) {
            timediff -= (start - pos)/speed;
            pos = start;
            return true;
        } else if (posnext >= end) {
            timediff -= (end - pos)/speed;
            pos = end;
            return true;
        } else {
            pos = posnext;
            timediff = 0.0f;
            return false;
        }
    }

    CellWall moveBallInOpenArea(glm::vec3 &position, float &timediff, glm::vec3 const &velocity) {
        glm::vec3 nextPos = position + velocity * timediff;
        std::array<float, 4> differences = {
                nextPos.x - m_componentSize/2,
                -m_componentSize/2 - nextPos.x,
                nextPos.y - m_componentSize/2,
                -m_componentSize/2 - nextPos.y
        };

        size_t largestDifferenceIndex = 0;
        for (size_t i = 1; i < differences.size(); i++) {
            if (differences[i] > differences[largestDifferenceIndex]) {
                largestDifferenceIndex = i;
            }
        }

        CellWall ret = CellWall::noWall;
        float timeDiffTillEdge = 0.0f;
        if (differences[largestDifferenceIndex] < 0.0f) {
            // movement within cell
            position = nextPos;
            timeDiffTillEdge = timediff;
        } else if (largestDifferenceIndex == 0) {
            timeDiffTillEdge = (m_componentSize/2 - position.x)/velocity.x;
            ret = CellWall::wallRight;
        } else if (largestDifferenceIndex == 1) {
            timeDiffTillEdge = (-m_componentSize/2 - position.x)/velocity.x;
            ret = CellWall::wallLeft;
        } else if (largestDifferenceIndex == 2) {
            timeDiffTillEdge = (m_componentSize/2 - position.y)/velocity.y;
            ret = CellWall::wallUp;
        } else if (largestDifferenceIndex == 3) {
            timeDiffTillEdge = (-m_componentSize/2 - position.y)/velocity.y;
            ret = CellWall::wallDown;
        } else {
            // shouldn't happen
            return CellWall::noWall;
        }
        if (timeDiffTillEdge < 0) {
            // shouldn't happen!
            timediff = 0.0f;
            return CellWall::noWall;
        }
        position += velocity * timeDiffTillEdge;
        timediff -= timeDiffTillEdge;
        return CellWall::noWall;
    }

    using MoveBallInCellFunc = std::function<Component::CellWall(glm::vec3 &, float &, glm::vec3 &)>;
    std::array<MoveBallInCellFunc, ComponentType::maxComponentAllowingBall + 1> const moveBallInCellFuncs = {
        // straight
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                position.x = 0.0f;
                bool ret = moveBall(position.y, timediff, -m_componentSize/2, m_componentSize/2, velocity.y);
                if (ret && position.y >= m_componentSize/2.0f) {
                    return CellWall::wallUp;
                } else if (ret && position.y <= -m_componentSize/2.0f) {
                    return CellWall::wallDown;
                } else {
                    return CellWall::noWall;
                }
            }),

        // T-Junction
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                if (position.y > 0.0f) {
                    position.y = 0.0f;
                }

                if (position.y == 0.0f && position.x == 0.0f) {
                    if (std::fabs(velocity.x) > std::fabs(velocity.y) || velocity.y > 0.0f) {
                        bool ret = moveBall(position.x, timediff, -m_componentSize/2,
                                m_componentSize/2, velocity.x);
                        if (ret && position.x <= -m_componentSize/2) {
                            return CellWall::wallLeft;
                        } else if (ret && position.x >= m_componentSize/2) {
                            return CellWall::wallRight;
                        } else {
                            return CellWall::noWall;
                        }
                    } else {
                        bool ret = moveBall(position.y, timediff, -m_componentSize/2, 0.0f, velocity.y);
                        if (ret && position.y <= -m_componentSize/2) {
                            return CellWall::wallDown;
                        } else {
                            return CellWall::noWall;
                        }
                    }
                }

                // bottom corridor
                if (position.y < 0.0f) {
                    position.x = 0.0f;
                    bool ret = moveBall(position.y, timediff, -m_componentSize/2, 0.0f, velocity.y);
                    if (ret && position.y == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallDown;
                    } else {
                        return CellWall::noWall;
                    }
                }

                // left corridor
                if (position.x < 0.0f) {
                    position.y = 0.0f;
                    bool ret = moveBall(position.x, timediff, -m_componentSize/2, 0.0f, velocity.x);
                    if (ret && position.x == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallLeft;
                    } else {
                        return CellWall::noWall;
                    }
                }

                // right corridor
                if (position.x > 0.0f) {
                    position.y = 0.0f;
                    bool ret = moveBall(position.x, timediff, 0.0f, m_componentSize/2, velocity.x);
                    if (ret && position.x == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallRight;
                    } else {
                        return CellWall::noWall;
                    }
                }
                return CellWall::noWall;
            }),

        // cross junction
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                if (position.y == 0.0f && position.x == 0.0f) {
                    if (std::fabs(velocity.x) > std::fabs(velocity.y)) {
                        bool ret = moveBall(position.x, timediff, -m_componentSize/2,
                                            m_componentSize/2, velocity.x);
                        if (ret && position.x <= -m_componentSize/2) {
                            return CellWall::wallLeft;
                        } else if (ret && position.x >= m_componentSize/2) {
                            return CellWall::wallRight;
                        } else {
                            return CellWall::noWall;
                        }
                    } else {
                        bool ret = moveBall(position.y, timediff, -m_componentSize/2, m_componentSize/2, velocity.y);
                        if (ret && position.y <= -m_componentSize/2) {
                            return CellWall::wallDown;
                        } else if (ret) {
                            return CellWall::wallUp;
                        } else {
                            return CellWall::noWall;
                        }
                    }
                }
                if (position.y < 0.0f) {
                    position.x = 0.0f;
                    bool ret = moveBall(position.y, timediff, -m_componentSize/2, 0.0f, velocity.y);
                    if (ret && position.y == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallDown;
                    } else {
                        return CellWall::noWall;
                    }
                }
                if (position.y > 0.0f) {
                    position.x = 0.0f;
                    bool ret = moveBall(position.y, timediff, 0.0f, m_componentSize/2, velocity.y);
                    if (ret && position.y == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallUp;
                    } else {
                        return CellWall::noWall;
                    }
                }
                if (position.x < 0.0f) {
                    position.y = 0.0f;
                    bool ret = moveBall(position.x, timediff, -m_componentSize/2, 0.0f, velocity.x);
                    if (ret && position.x == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallLeft;
                    } else {
                        return CellWall::noWall;
                    }
                }
                if (position.x > 0.0f) {
                    position.y = 0.0f;
                    bool ret = moveBall(position.x, timediff, 0.0f, m_componentSize/2, velocity.x);
                    if (ret && position.x == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallRight;
                    } else {
                        return CellWall::noWall;
                    }
                }

                return CellWall::noWall;
            }),

        // turn
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                if (position.y > 0.0f) {
                    position.y = 0.0f;
                }
                if (position.x > 0.0f) {
                    position.x = 0.0f;
                }

                if (position.y == 0.0f && position.x == 0.0f) {
                    if (std::fabs(velocity.x) > std::fabs(velocity.y) || velocity.y > 0.0f) {
                        bool ret = moveBall(position.x, timediff, -m_componentSize/2,
                                            m_componentSize/2, velocity.x);
                        if (ret && position.x <= -m_componentSize/2) {
                            return CellWall::wallLeft;
                        } else if (ret && position.x >= m_componentSize/2) {
                            return CellWall::wallRight;
                        } else {
                            return CellWall::noWall;
                        }
                    } else {
                        bool ret = moveBall(position.y, timediff, -m_componentSize/2, 0.0f, velocity.y);
                        if (ret && position.y <= -m_componentSize/2) {
                            return CellWall::wallDown;
                        } else {
                            return CellWall::noWall;
                        }
                    }
                }

                // bottom corridor
                if (position.y < 0.0f) {
                    position.x = 0.0f;
                    bool ret = moveBall(position.y, timediff, -m_componentSize/2, 0.0f, velocity.y);
                    if (ret && position.y == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallDown;
                    } else {
                        return CellWall::noWall;
                    }
                }

                // left corridor
                if (position.x < 0.0f) {
                    position.y = 0.0f;
                    bool ret = moveBall(position.x, timediff, 0.0f, m_componentSize/2, velocity.x);
                    if (ret && position.x == 0.0f) {
                        return moveBallInCellFuncs[ComponentType::tjunction](position, timediff, velocity);
                    } else if (ret) {
                        return CellWall::wallRight;
                    } else {
                        return CellWall::noWall;
                    }
                }

                return CellWall::noWall;
            }),

        // open area
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                return moveBallInOpenArea(position, timediff, velocity);
            }),

        // closed bottom
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                CellWall wall = moveBallInOpenArea(position, timediff, velocity);
                if (wall == CellWall::wallDown) {
                    timediff = 0.0f;
                    velocity = {0.0f, 0.0f, 0.0f};
                    return CellWall::noWall;
                }
                return wall;
            }),

        // closed corner
        MoveBallInCellFunc(
            [&](glm::vec3 &position, float &timediff, glm::vec3 &velocity) -> Component::CellWall {
                CellWall wall = moveBallInOpenArea(position, timediff, velocity);
                if (wall == CellWall::wallDown || wall == CellWall::wallLeft) {
                    timediff = 0.0f;
                    velocity = {0.0f, 0.0f, 0.0f};
                    return CellWall::noWall;
                }
                return wall;
            })
    };

    class Placement {
    public:
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
            m_movePositionSoFar{0.0f, 0.0f}
        {
        }

        /* accessors */
        uint32_t row() { return m_row; }
        uint32_t col() { return m_col; }
        float rotationAngle () { return m_rotationAngle; }
        bool lockedIntoPlace() { return m_lockedIntoPlace; }
        auto &prev() { return m_prev; }
        auto &next() { return m_next; }
        glm::vec2 movePositionSoFar() { return m_movePositionSoFar; }

        /* setters */
        void setRC(uint32_t row, uint32_t col) { m_row = row; m_col = col; }
        void rotate() { m_rotationAngle += glm::radians(90.0f); }

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
    private:
        uint32_t m_row;
        uint32_t m_col;
        float m_rotationAngle; /*radians */
        bool m_lockedIntoPlace;
        std::pair<std::shared_ptr<Component>, size_t> m_prev;
        std::pair<std::shared_ptr<Component>, size_t> m_next;
        bool m_moveInProgress;
        glm::vec2 m_movePositionSoFar;
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

    bool operator==(Component const &other) { return other.m_componentType == m_componentType; }
    bool operator<(Component const &other) { return m_componentType < other.m_componentType; }
    auto placementsBegin() { return m_placements.begin(); }
    auto placementsEnd() { return m_placements.end(); }
    auto type() { return m_componentType; }
    Placement &placement(size_t index) { return m_placements[index]; }
    size_t nbrPlacements() { return m_placements.size(); }
    float componentSize() { return m_componentSize; }

    void setSize(float tileSize) { m_componentSize = tileSize; }
    void setObjReference(MovableType objType, uint32_t ref) {
        m_objReference.first = objType;
        m_objReference.second = ref;
    }

    std::pair<MovableType, uint32_t> objReference() { return m_objReference; }

    Component(ComponentType inType, float componentSize = 0.0f)
        : m_componentType{inType},
        m_componentSize{componentSize},
        m_objReference{MovableType::noneAssigned, 0}
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
    std::pair<MovableType, uint32_t> m_objReference;
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
    std::shared_ptr<Component> &component() { return m_component; }
    size_t placementIndex() { return m_placementIndex; }

    void setBlockType(BlockType blockType) { m_blockType = blockType; }

    void setComponent(std::shared_ptr<Component> component, size_t placementIndex) {
        m_component = std::move(component);
        m_placementIndex = placementIndex;
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
};

class GameBoard {
public:
    static uint32_t constexpr m_nbrTileRowsForStart = 4;
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
            //z = m_centerPos.z - m_blockSize / 2.0f;
            z = m_centerPos.z - 3*m_blockSize/4.0f ;
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
    glm::vec4 getBackgroundColor() override { return {1.0f, 1.0f, 1.0f, 1.0f}; };
    bool updateData() override;
    bool updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) override;
    bool updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) override;
    void start() override { m_prevTime = std::chrono::high_resolution_clock::now(); }
    void getLevelFinisherCenter(float &x, float &y) override { x = 0.0f; y = 0.0f; };
    SaveLevelDataFcn getSaveLevelDataFcn() override;

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
            std::string const &blockedRockTexture,
            std::string const &blockedDirtTexture,
            std::string const &componentTextureEnd,
            std::string const &textureEndOffBoard,
            std::string const &startCornerModel,
            std::string const &startCornerTexture,
            std::string const &startSideModel,
            std::string const &startSideTexture,
            std::string const &startOpenModel,
            std::string const &startOpenTexture)
    {
        m_componentTextures[Component::ComponentType::noMovementRock] = blockedRockTexture;
        m_componentTextures[Component::ComponentType::noMovementDirt] = blockedDirtTexture;

        m_componentTextureEnd = componentTextureEnd;
        m_textureEndOffBoard = textureEndOffBoard;

        m_componentModels[Component::ComponentType::closedCorner] = startCornerModel;
        m_componentTextures[Component::ComponentType::closedCorner] = startCornerTexture;

        m_componentModels[Component::ComponentType::closedBottom] = startSideModel;
        m_componentTextures[Component::ComponentType::closedBottom] = startSideTexture;

        m_componentModels[Component::ComponentType::open] = startOpenModel;
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
            m_componentTextures[inComponentType] = texture;
        }
        if (!model.empty()) {
            m_componentModels[inComponentType] = model;
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

    std::array<std::string, Component::ComponentType::maxComponentType + 1> m_componentModels;
    std::array<std::string, Component::ComponentType::maxComponentType + 1> m_componentTextures;
    std::string m_componentTextureEnd;
    std::string m_textureEndOffBoard;

    // rocks row and col do not consider the off game squares or the start or end.  Just
    // the middle of the game board.  (0,0) is at the bottom left part of the board.
    std::vector<std::pair<uint32_t, uint32_t>> m_addedRocks;

    // the row and column which starts the area in which users can place blocks
    std::pair<uint32_t, uint32_t> m_gameBoardStartRowColumn;

    // the row and column which ends the area in which users can place blocks
    std::pair<uint32_t, uint32_t> m_gameBoardEndRowColumn;

    // the column in which the end tile is located.
    uint32_t m_columnEndPosition;
};
#endif /* AMAZING_LABYRINTH_MOVABLE_PASSAGE_HPP */
