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

#include "movablePassageAlgorithms.hpp"

std::pair<uint32_t, uint32_t> GameBoard::findRC(glm::vec2 position) {
    uint32_t row;
    uint32_t col;

    row = static_cast<uint32_t>(std::floor(((position.y - m_centerPos.y)/m_height + 0.5f) * m_blocks.size()));
    col = static_cast<uint32_t>(std::floor(((position.x - m_centerPos.x)/m_width + 0.5f) * m_blocks[0].size()));

    return std::make_pair(row, col);
}

// returs true if a redraw is needed.
// expects the position and distance in world space.
// the distance is relative to the previous moved position, not relative to the start position.
bool GameBoard::drag(glm::vec2 const &startPosition, glm::vec2 const &distance) {
    std::pair<uint32_t, uint32_t> rc = findRC(startPosition);
    if (m_moveInProgress) {
        if (rc.first != m_moveStartingPosition.first || rc.second != m_moveStartingPosition.second) {
            // a move started, but the previous one never completed.  Just move the old piece back
            // in place and start the new move
            auto &b = m_blocks[m_moveStartingPosition.first][m_moveStartingPosition.second];
            b.component()->placement(b.placementIndex()).moveDone();
            m_moveInProgress = false;
        }
    }
    auto &b = m_blocks[rc.first][rc.second];
    glm::vec2 totalDistance = distance;
    if (!m_moveInProgress) {
        if (hasMovableComponent(b)) {
            auto component = b.component();
            auto &placement = component->placement(b.placementIndex());
            if (!placement.lockedIntoPlace() && placement.prev().first == nullptr) {
                m_moveInProgress = true;
                m_moveStartingPosition = rc;
                totalDistance += startPosition;
            }
        } else {
            return false;
        }
    }
    if (m_moveInProgress) {
        b.component()->placement(b.placementIndex()).movePlacement(totalDistance);
    }
    return true;
}

// returs true if a redraw is needed.
// expects the position in world space.
bool GameBoard::dragEnded(glm::vec2 const &endPosition) {
    if (!m_moveInProgress) {
        return false;
    }

    std::pair<uint32_t, uint32_t> rc = findRC(endPosition);
    auto &b = m_blocks[m_moveStartingPosition.first][m_moveStartingPosition.second];
    if (rc.first == m_moveStartingPosition.first && rc.second == m_moveStartingPosition.second) {
        // moving to the same position we started at. fail the move.
        m_moveInProgress = false;
        b.component()->placement(b.placementIndex()).moveDone();

        // we still need to redraw even if the move failed.  to move the component back to the
        // original spot.
        return true;
    }

    auto &bEnd = m_blocks[rc.first][rc.second];
    if ((bEnd.component()->type() == Component::ComponentType::noMovementDirt &&
         bEnd.blockType() == GameBoardBlock::BlockType::onBoard) ||
        (bEnd.component()->type() == Component::ComponentType::noMovementRock &&
         bEnd.blockType() == GameBoardBlock::BlockType::offBoard))
    {
        // a move was started and it succeeded.  It needs to be completed now.
        auto tmp = b.component();
        auto tmpIndex = b.placementIndex();
        auto tmpEnd = bEnd.component();
        auto tmpEndIndex = bEnd.placementIndex();
        tmp->placement(tmpIndex).moveDone();
        tmp->placement(tmpIndex).setRC(rc.first, rc.second);
        bEnd.setComponent(tmp, tmpIndex);
        bEnd.setSecondaryComponent(tmpEnd, tmpEndIndex);
        b.setComponent(b.secondaryComponent(), b.secondaryPlacementIndex());
        b.setSecondaryComponent(nullptr, 0);
        m_moveInProgress = false;
        return true;
    }

    // move failed - there is a non-dirt component at the spot to be moved to,
    // or the target spot is end or start space on the board.
    m_moveInProgress = false;
    b.component()->placement(b.placementIndex()).moveDone();

    // we still need to redraw even if the move failed.  to move the component back to the
    // original spot.
    return true;
}

bool GameBoard::tap(glm::vec2 const &position) {
    std::pair<uint32_t, uint32_t> rc = findRC(position);
    auto &b = m_blocks[rc.first][rc.second];
    if (!hasMovableComponent(b)) {
        return false;
    }
    auto &placement = b.component()->placement(b.placementIndex());
    if (placement.lockedIntoPlace() || placement.prev().first != nullptr) {
        return false;
    }
    placement.rotate();
    return true;
}

std::pair<bool, bool> GameBoard::checkforNextWall(Component::CellWall wall1, Component::CellWall wall2,
                                                  uint32_t ballRow, uint32_t ballCol)
{
    auto hasWallAt = [&](std::pair<size_t, size_t> rc, Component::CellWall cellWall) -> bool {
        auto &b = block(rc.first, rc.second);
        if (b.blockType() == GameBoardBlock::BlockType::end) {
            return false;
        }
        if (b.blockType() == GameBoardBlock::BlockType::offBoard) {
            return true;
        }
        return b.component()->hasWallAt(cellWall, b.placementIndex());
    };

    size_t width = widthInTiles();
    size_t height = heightInTiles();
    auto nextRC = [width, height](Component::CellWall cellWall, size_t row, size_t col) -> std::pair<size_t, size_t> {
        if (cellWall == Component::CellWall::wallLeft) {
            if (col != 0) {
                col--;
            }
        } else if (cellWall == Component::CellWall::wallRight) {
            if (col != width - 1) {
                col++;
            }
        } else if (cellWall == Component::CellWall::wallDown) {
            if (row != 0) {
                row--;
            }
        } else if (cellWall == Component::CellWall::wallUp) {
            if (row != height - 1) {
                row++;
            }
        }
        return std::make_pair(row, col);
    };

    auto nextComponentWall = [](Component::CellWall cellWall) -> Component::CellWall {
        switch (cellWall) {
            case Component::CellWall::wallLeft:
                return Component::CellWall::wallRight;
            case Component::CellWall::wallRight:
                return Component::CellWall::wallLeft;
            case Component::CellWall::wallDown:
                return Component::CellWall::wallUp;
            case Component::CellWall::wallUp:
                return Component::CellWall::wallDown;
            case Component::CellWall::noWall:
                return Component::CellWall::noWall;
        }
    };

    auto checkRC = [] (std::pair<size_t, size_t> rc, std::pair<size_t, size_t> rc1) {
        return rc.first != rc1.first || rc.second != rc1.second;
    };

    auto &bOld = block(ballRow, ballCol);
    Component::CellWall wall1Actual = bOld.component()->actualWall(wall1, bOld.placementIndex());
    Component::CellWall wall2Actual = bOld.component()->actualWall(wall2, bOld.placementIndex());
    auto rc1 = nextRC(wall1Actual, ballRow, ballCol);
    auto rc2 = nextRC(wall2Actual, ballRow, ballCol);
    auto rcOld = std::make_pair(ballRow, ballCol);
    if (wall1Actual != Component::CellWall::noWall && wall2Actual != Component::CellWall::noWall) {
        // diagonal move.  try both paths to get to the diagonal cell, if either path
        // works, return <true, true>
        if (checkRC(rc1, rcOld)) {
            // path 1: wall 1 then wall 2.
            if (!hasWallAt(rc1, nextComponentWall(wall1Actual))) {
                auto rcDiagonal = nextRC(wall2Actual, rc1.first, rc1.second);
                if (checkRC(rcDiagonal, rc1) &&
                    !hasWallAt(rcDiagonal, nextComponentWall(wall2Actual)))
                {
                    return std::make_pair(true, true);
                }
            }
        }
        if (checkRC(rc2, rcOld)) {
            // path 2: wall 2, then wall 1.
            if (!hasWallAt(rc2, nextComponentWall(wall2Actual))) {
                auto rcDiagonal = nextRC(wall1Actual, rc2.first, rc2.second);
                if (checkRC(rcDiagonal, rc2) &&
                    !hasWallAt(rcDiagonal, nextComponentWall(wall1Actual)))
                {
                    return std::make_pair(true, true);
                }
            }
        }
    }
    if (wall1Actual != Component::CellWall::noWall && checkRC(rc1, rcOld) &&
        !hasWallAt(rc1, nextComponentWall(wall1Actual)))
    {
        return std::make_pair(true, false);
    } else  if (wall2Actual != Component::CellWall::noWall && checkRC(rc2, rcOld) &&
                !hasWallAt(rc2, nextComponentWall(wall2Actual)))
    {
        return std::make_pair(false, true);
    } else {
        return std::make_pair(false, false);
    }
}

size_t chooseObj(
        Random &randomNumbers,
        std::shared_ptr<Component> const &component,
        size_t placementIndex)
{
    std::vector<size_t> refs;
    if (!component->placement(placementIndex).movementAllowed()) {
        refs = component->dynObjReferencesLockedComponent();
    }
    if (refs.size() == 0) {
        refs = component->dynObjReferences();
    }

    size_t i;
    switch (refs.size()) {
        case 0:
            return Component::Placement::m_invalidObjReference;
        case 1:
            i = 0;
            break;
        default:
            i = randomNumbers.getUInt(0, refs.size() - 1);
    }

    if (component) {
        component->placement(placementIndex).setDynObjReference(refs[i]);
    }
    return refs[i];
}

size_t addModelMatrixToObj(
        Random &randomNumbers,
        DrawObjectTable &objs,
        std::vector<size_t> const &refs,
        std::shared_ptr<Component> const &component,
        size_t placementIndex,
        glm::mat4 modelMatrix)
{
    size_t i;
    switch (refs.size()) {
        case 0:
            return Component::Placement::m_invalidObjReference;
        case 1:
            i = 0;
        default:
            i = randomNumbers.getUInt(0, refs.size() - 1);
    }

    objs[refs[i]].first->modelMatrices.insert(objs[refs[i]].first->modelMatrices.begin(), modelMatrix);
    if (component) {
        component->placement(placementIndex).setDynObjReference(refs[i]);
    }
    return refs[i];
}

void blockUnblockPlacements(
        std::shared_ptr<Component> const &oldComponent,
        size_t oldPlacementIndex,
        std::shared_ptr<Component> const &newComponent,
        size_t newPlacementIndex)
{
    Component::Placement &oldPlacement = oldComponent->placement(oldPlacementIndex);
    Component::Placement &newPlacement = newComponent->placement(newPlacementIndex);
    if (newPlacement.next().first != nullptr) {
        if (newPlacement.next().first == oldComponent &&
                newPlacement.next().second == oldPlacementIndex) {
            // We are going backwards.  Unblock block that we were at and move on.
            oldPlacement.prev() = std::make_pair(nullptr, 0);
            newPlacement.next() = std::make_pair(nullptr, 0);

            // remove the obj reference so that the obj reference for the locked placement
            // can be added.
            oldPlacement.setDynObjReference(Component::Placement::m_invalidObjReference);
        } else {
            // We encountered a loop.  Unblock the entire loop
            std::shared_ptr<Component> nextComponent = newComponent;
            size_t index = newPlacementIndex;
            std::shared_ptr<Component> loopComponent = oldComponent;
            size_t loopIndex = oldPlacementIndex;
            while (loopComponent != nullptr &&
                   (loopComponent != nextComponent || loopIndex != index))
            {
                auto tmp = loopComponent->placement(loopIndex).prev();
                auto &loopPlacement = loopComponent->placement(loopIndex);
                loopPlacement.prev() = std::make_pair(nullptr, 0);
                loopPlacement.next() = std::make_pair(nullptr, 0);
                loopPlacement.setDynObjReference(Component::Placement::m_invalidObjReference);
                loopComponent = tmp.first;
                loopIndex = tmp.second;
            }
            loopComponent->placement(loopIndex).next() = std::make_pair(nullptr, 0);
        }
    } else {
        // the ball is entering a new cell that is not in its path yet.
        newPlacement.prev() = std::make_pair(oldComponent, oldPlacementIndex);
        newPlacement.setDynObjReference(Component::Placement::m_invalidObjReference);

        oldPlacement.next() = std::make_pair(newComponent, newPlacementIndex);
    }

}
