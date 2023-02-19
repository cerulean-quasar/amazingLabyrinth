/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

#ifndef AMAZING_LABYRINTH_MOVABLE_PASSAGE_ALGORITHMS_HPP
#define AMAZING_LABYRINTH_MOVABLE_PASSAGE_ALGORITHMS_HPP

#include <vector>
#include <array>
#include <set>
#include <memory>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/optional.hpp>

#include "basic/level.hpp"
#include "../random.hpp"

// the model index and the texture index are indices into the set of models name and texture names.
// they are needed for saving the game data.  This way the game will be restored with the same
// textures and models for all the components.  We also need to know whether the texture set was
// from the locked in place models/textures or the non-locked in place models/textures.
// The objIsDynAndIndex indicates if it is a dynamic reference (true) and what the index is.
struct ObjReference {
    // The reference used to identify the draw object.
    boost::optional<levelDrawer::DrawObjReference> objRef;

    bool isLockedInPlaceRef;
    size_t modelIndex;
    size_t textureIndex;

    ObjReference(
            levelDrawer::DrawObjReference objRef_,
            bool isLockedInPlaceRef_,
            size_t modelIndex_,
            size_t textureIndex_)
            : objRef{objRef_},
              isLockedInPlaceRef{isLockedInPlaceRef_},
              modelIndex{modelIndex_},
              textureIndex{textureIndex_}
    {}

    ObjReference(
            bool isLockedInPlaceRef_,
            size_t modelIndex_,
            size_t textureIndex_)
            : objRef{boost::none},
              isLockedInPlaceRef{isLockedInPlaceRef_},
              modelIndex{modelIndex_},
              textureIndex{textureIndex_}
    {}

    ObjReference()
        : objRef{boost::none},
          isLockedInPlaceRef{false},
          modelIndex{0},
          textureIndex{0}
    {}

    ObjReference(ObjReference &&other) = default;

    ObjReference(ObjReference const &other) = default;

    ObjReference &operator=(ObjReference const &other) = default;

    ObjReference &operator=(ObjReference &&other) = default;

    bool operator==(ObjReference const &other) const {
        return isLockedInPlaceRef == other.isLockedInPlaceRef &&
                modelIndex == other.modelIndex &&
                textureIndex == other.textureIndex;
    }

    bool operator<(ObjReference const &other) const {
        if (isLockedInPlaceRef != other.isLockedInPlaceRef) {
            return isLockedInPlaceRef;
        }
        return textureIndex < other.textureIndex || modelIndex < other.modelIndex;
    }
};

class Component {
public:
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
        deadEnd,
        maxComponentJunctionType = 4,
        open,
        closedBottom,
        closedCorner,
        maxComponentAllowingBall = 7,
        noMovementDirt,
        noMovementRock,
        maxComponentType = 9
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
                p = border + sign * basic::Level::m_floatErrorAmount;
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

        if (fabs(nextPos.x - position.x) > fabs(nextPos.y - position.y)) {
            auto ret = moveBallCorridor(nextPos.x, velocity.x,
                                        CellWall::wallLeft, -m_componentSize/2,
                                        CellWall::wallRight, m_componentSize/2);
            if (fabs(nextPos.x) > ballRadius/4) {
                nextPos.y = 0.0f;
                velocity.y = 0.0f;
            }
            position = nextPos;
            return ret;
        } else {
            auto ret = moveBallCorridor(nextPos.y, velocity.y,
                                        CellWall::wallDown, -m_componentSize/2,
                                        CellWall::wallUp, m_componentSize/2);
            if (fabs(nextPos.y) > ballRadius/4) {
                nextPos.x = 0.0f;
                velocity.x = 0.0f;
            }
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
                fabs(velocity.x) < basic::Level::m_floatErrorAmount ? -1 : (rightWall - position.x)/velocity.x,
                fabs(velocity.x) < basic::Level::m_floatErrorAmount ? -1 : (leftWall - position.x)/velocity.x,
                fabs(velocity.y) < basic::Level::m_floatErrorAmount ? -1 : (topWall - position.y)/velocity.y,
                fabs(velocity.y) < basic::Level::m_floatErrorAmount ? -1 : (bottomWall - position.y)/velocity.y
        };

        size_t largestDifferenceIndex = differences.size();
        for (size_t i = 0; i < differences.size(); i++) {
            if (differences[i] < basic::Level::m_floatErrorAmount) {
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
                pos = wallPos + sign * basic::Level::m_floatErrorAmount;
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
                            if (velocity.y > 0.0f) {
                                velocity.y = 0.0f;
                            }
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
                            if (velocity.y > 0.0f) {
                                velocity.y = 0.0f;
                            }
                        }
                        if (nextPos.x > 0.0f || position.x > 0.0f) {
                            position.x = 0.0f;
                            if (velocity.x > 0.0f) {
                                velocity.x = 0.0f;
                            }
                        }

                        return moveBallInJunction(position, timediff, velocity, ballRadius, checkForNextWall);
                    }),

            // dead end
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
                            if (velocity.y > 0.0f) {
                                velocity.y = 0.0f;
                            }
                        }

                        position.x = 0.0f;
                        velocity.x = 0.0f;

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
        /* accessors */
        uint32_t row() { return m_row; }
        uint32_t col() { return m_col; }
        float rotationAngle () { return m_nbr90DegreeRotations * glm::radians(90.0f); }
        uint32_t nbr90DegreeRotations() { return m_nbr90DegreeRotations; }
        bool lockedIntoPlace() { return m_lockedIntoPlace; }
        auto &prev() { return m_prev; }
        auto &next() { return m_next; }
        glm::vec2 movePositionSoFar() { return m_movePositionSoFar; }
        boost::optional<ObjReference> objReference() { return m_objReference; }
        boost::optional<levelDrawer::DrawObjDataReference> objDataReference() { return m_objDataReference; }
        bool movementAllowed() { return (!m_lockedIntoPlace) && (m_prev.first == nullptr); }

        /* setters */
        void setRC(uint32_t row, uint32_t col) { m_row = row; m_col = col; }
        void setObjAndDataReference(
                boost::optional<ObjReference> ref,
                boost::optional<levelDrawer::DrawObjDataReference> dataRef)
        {
            m_objReference = std::move(ref);
            m_objDataReference = std::move(dataRef);
        }

        void setLockedIntoPlace(bool lockedIntoPlace) { m_lockedIntoPlace = lockedIntoPlace; }
        void rotate() {
            m_nbr90DegreeRotations = (m_nbr90DegreeRotations + 1) %4;
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
                  uint32_t nbr90DegreeRotations = 0,
                  bool lockedIntoPlace = false,
                  std::shared_ptr<Component> prevComponent = nullptr,
                  size_t prevIndex = 0,
                  std::shared_ptr<Component> nextComponent = nullptr,
                  size_t nextIndex = 0)
                : m_row{row},
                  m_col{col},
                  m_nbr90DegreeRotations{nbr90DegreeRotations},
                  m_lockedIntoPlace{lockedIntoPlace},
                  m_prev{std::make_pair(prevComponent, prevIndex)},
                  m_next{std::make_pair(nextComponent, nextIndex)},
                  m_moveInProgress{false},
                  m_movePositionSoFar{0.0f, 0.0f},
                  m_objReference{boost::none}
        {
        }
    private:
        uint32_t m_row;
        uint32_t m_col;
        uint32_t m_nbr90DegreeRotations;
        bool m_lockedIntoPlace;
        std::pair<std::shared_ptr<Component>, size_t> m_prev;
        std::pair<std::shared_ptr<Component>, size_t> m_next;
        bool m_moveInProgress;
        glm::vec2 m_movePositionSoFar;
        boost::optional<ObjReference> m_objReference;
        boost::optional<levelDrawer::DrawObjDataReference> m_objDataReference;
    };

    uint32_t add(
            uint32_t row = 0,
            uint32_t col = 0,
            uint32_t nbr90DegreeRotations = 0,
            bool lockedIntoPlace = false,
            std::shared_ptr<Component> prevComponent = nullptr,
            size_t prevIndex = 0,
            std::shared_ptr<Component> nextComponent = nullptr,
            size_t nextIndex = 0)
    {
        m_placements.emplace_back(row, col, nbr90DegreeRotations, lockedIntoPlace,
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
    std::set<ObjReference> objReferences() { return m_objReferences; }
    std::set<ObjReference> objReferencesLockedComponent() { return m_objReferencesLockedComponent; }

    void setSize(float tileSize) { m_componentSize = tileSize; }
    void setObjReferences(std::set<ObjReference> refs) { m_objReferences = refs; }
    void setObjReferencesLockedComponent(std::set<ObjReference> refs) { m_objReferencesLockedComponent = refs; }

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
            case ComponentType::deadEnd:
                m_cellWalls.insert(CellWall::wallLeft);
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
    std::set<ObjReference> m_objReferences;
    std::set<ObjReference> m_objReferencesLockedComponent;
};

// The game board is the section of the surface in which components can be placed.  There are
// off board segments that can contain components where the ball cannot go.  On board is the
// areas where the ball can go.  The game board does not include the blocks around the edges
// to fill in empty area because the device surface is not an integer number of blocks.  The
// end is special, not really part of the game board.  On board segments are the game board
// segments where the ball is allowed to go and that the user can place components
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
    uint32_t widthInTiles() { return m_blocks.empty() ? 0 : m_blocks[0].size(); }
    uint32_t heightInTiles() { return m_blocks.size(); }
    bool drag(
            levelDrawer::Adaptor &levelDrawer,
            Random &randomNumbers,
            float modelSize,
            float zMovingPlacement,
            float offBoardComponentScaleFactor,
            glm::vec2 const &startPosition,
            glm::vec2 const &distance);
    bool dragEnded(
            levelDrawer::Adaptor &levelDrawer,
            float offBoardComponentScaleMultiplier,
            float modelSize,
            glm::vec2 const &endPosition);
    bool tap(
            levelDrawer::Adaptor &levelDrawer,
            float offBoardComponentScaleMultiplier,
            float modelSize,
            glm::vec2 const &positionOfTap);
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

    void initialize(
            float tileSize,
            glm::vec3 const &pos,
            uint32_t rows,
            uint32_t cols,
            uint32_t nbrTileRowsForStart,
            uint32_t nbrTileRowsForEnd)
    {
        m_nbrTileRowsForStart = nbrTileRowsForStart;
        m_nbrTileRowsForEnd = nbrTileRowsForEnd;
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
            : m_nbrTileRowsForStart{0},
              m_nbrTileRowsForEnd{0},
              m_width{0.0f},
              m_height{0.0f},
              m_centerPos{0.0f, 0.0f, 0.0f},
              m_blockSize{0.0f},
              m_moveInProgress{false},
              m_moveStartingPosition{std::make_pair(0,0)}
    {}
private:
    uint32_t m_nbrTileRowsForStart;
    uint32_t m_nbrTileRowsForEnd;

    float m_width;
    float m_height;

    // expected to be set to the x, y center of the game board and z is set to the where
    // the tops of the components should go.
    glm::vec3 m_centerPos;

    float m_blockSize;
    std::vector<std::vector<GameBoardBlock>> m_blocks;

    bool m_moveInProgress;
    std::pair<uint32_t, uint32_t> m_moveStartingPosition;

    void drawPlacements(
            levelDrawer::Adaptor &levelDrawer,
            float offBoardComponentScaleMultiplier,
            float modelSize,
            size_t row,
            size_t col);

    bool hasMovableComponent(GameBoardBlock &b) {
        return (b.blockType() == GameBoardBlock::BlockType::offBoard ||
                b.blockType() == GameBoardBlock::BlockType::onBoard) &&
               b.component() != nullptr &&
               b.component()->type() != Component::ComponentType::noMovementRock &&
               b.component()->type() != Component::ComponentType::noMovementDirt;
    }
};

template <typename AlternateModelDescriptionType>
std::set<ObjReference> addObjs(
        levelDrawer::Adaptor &levelDrawer,
        bool isLockedInPlaceRef,
        std::vector<std::string> const &models,
        std::vector<std::string> const &textureNames)
{
    std::set<ObjReference> ret;
    std::vector<std::shared_ptr<levelDrawer::ModelDescription>> v;
    if (models.empty()) {
        v.push_back(std::make_shared<AlternateModelDescriptionType>());
    } else {
        for (size_t i = 0; i < models.size(); i++) {
            v.push_back(std::make_shared<levelDrawer::ModelDescriptionPath>(models[i]));
        }
    }

    for (size_t i = 0; i < std::max(v.size(), textureNames.size()); i++) {
        auto objRef = levelDrawer.addObject(
                v[i%v.size()],
                std::make_shared<levelDrawer::TextureDescriptionPath>(
                        textureNames[i%textureNames.size()]));

        ret.emplace(objRef, isLockedInPlaceRef, i % v.size(), i % textureNames.size());
    }

    return std::move(ret);
}

std::pair<boost::optional<ObjReference>, boost::optional<levelDrawer::DrawObjDataReference>> chooseObj(
        levelDrawer::Adaptor &levelDrawer,
        Random &randomNumbers,
        std::shared_ptr<Component> const &component,
        size_t placementIndex,
        glm::mat4 const &modelMatrix);

std::pair<boost::optional<ObjReference>, boost::optional<levelDrawer::DrawObjDataReference>> addModelMatrixToObj(
        levelDrawer::Adaptor &levelDrawer,
        Random &randomNumbers,
        std::set<ObjReference> const &refs,
        std::shared_ptr<Component> const &component,
        size_t placementIndex,
        glm::mat4 modelMatrix);

void blockUnblockPlacements(
        levelDrawer::Adaptor &levelDrawer,
        Random &randomNumbers,
        GameBoard &gameBoard,
        float modelSize,
        std::shared_ptr<Component> const &oldComponent,
        size_t oldPlacementIndex,
        std::shared_ptr<Component> const &newComponent,
        size_t newPlacementIndex);

// restore the path that the ball went through
void restorePathLockedInPlace(GameBoard &gameBoard, std::vector<Point<uint32_t>> const &pathLockedInPlace);

#endif // AMAZING_LABYRINTH_MOVABLE_PASSAGE_ALGORITHMS_HPP
