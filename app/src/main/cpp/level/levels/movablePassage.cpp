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
#include "movablePassage.hpp"
#include "../level.hpp"

std::pair<uint32_t, uint32_t> GameBoard::findRC(glm::vec2 position) {
    uint32_t row;
    uint32_t col;

    row = static_cast<uint32_t>(std::floor((position.y/m_height + 0.5f) * m_blocks.size()));
    col = static_cast<uint32_t>(std::floor((position.x/m_width + 0.5f) * m_blocks[0].size()));

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
            m_moveStartingPosition = rc;
            m_moveInProgress = false;
        }
    }
    auto &b = m_blocks[rc.first][rc.second];
    if (!m_moveInProgress) {
        if (hasMovableComponent(b)) {
            auto component = b.component();
            auto &placement = component->placement(b.placementIndex());
            if (!placement.lockedIntoPlace() && placement.prev().first == nullptr) {
                m_moveInProgress = true;
            }
        } else {
            return false;
        }
    }
    if (m_moveInProgress) {
        b.component()->placement(b.placementIndex()).movePlacement(distance);
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
    auto &bEnd = m_blocks[rc.first][rc.second];
    if ((bEnd.component() != nullptr &&
         bEnd.component()->type() == Component::ComponentType::noMovementDirt &&
         bEnd.blockType() == GameBoardBlock::BlockType::onBoard) ||
        (bEnd.component() != nullptr &&
         bEnd.component()->type() == Component::ComponentType::noMovementRock &&
         bEnd.blockType() != GameBoardBlock::BlockType::offBoard))
    {
        // a move was started and it succeeded.  It needs to be completed now.
        auto &b = m_blocks[m_moveStartingPosition.first][m_moveStartingPosition.second];
        b.component()->placement(b.placementIndex()).moveDone();
        b.component()->placement(b.placementIndex()).setRC(rc.first, rc.second);
        bEnd.component()->placement(bEnd.placementIndex()).setRC(m_moveStartingPosition.first,
                                                                 m_moveStartingPosition.second);
        auto tmp = b.component();
        auto tmpIndex = b.placementIndex();
        b.setComponent(bEnd.component(), bEnd.placementIndex());
        bEnd.setComponent(tmp, tmpIndex);
        m_moveInProgress = false;
        return true;
    }

    // move failed - there is a non-dirt component at the spot to be moved to,
    // or the target spot is end or start space on the board.
    bEnd.component()->placement(bEnd.placementIndex()).moveDone();

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
    if (placement.lockedIntoPlace() && placement.prev().first != nullptr) {
        return false;
    }
    placement.rotate();
    return true;
}

void MovablePassage::initSetGameBoard(
        uint32_t nbrTilesX,
        uint32_t nbrTilesY,
        uint32_t startColumn,
        uint32_t endColumn) {
    float tileSizeX = m_width / nbrTilesX;
    float tileSizeY = m_height / (nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                  GameBoard::m_nbrTileRowsForEnd);
    float tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;
    uint32_t nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
            (m_width - tileSize * nbrTilesX) / tileSize));
    uint32_t nbrExtraTilesX = nbrExtraTileRowsX * nbrTilesY;

    // minus 2 * nbrExtraTileRowsY for up tunnel components
    uint32_t nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
            (m_height - tileSize * (nbrTilesY +
                    GameBoard::m_nbrTileRowsForEnd + GameBoard::m_nbrTileRowsForStart)) / tileSize));
    uint32_t nbrExtraTilesY = nbrExtraTileRowsY * nbrTilesX - 2 * nbrExtraTileRowsY;

    // we have a device with a screen of a similar size to our tunnel building area,
    // get some more space by making the movable space less.
    if (nbrExtraTilesX + nbrExtraTilesY < m_nbrComponents) {
        uint32_t moreExtraComponentsRequired = m_nbrComponents - (nbrExtraTilesX + nbrExtraTilesY);
        uint32_t nbrExtraPerimeters =
                moreExtraComponentsRequired / (2 * nbrTilesX + 2 * nbrTilesY) + 1;
        tileSizeX = m_width / (nbrTilesX + 2 * nbrExtraPerimeters);
        tileSizeY = m_height / (nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                GameBoard::m_nbrTileRowsForEnd + 2 * nbrExtraPerimeters);
        tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;

        nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                m_width/tileSize - nbrTilesX));

        // minus 2 * nbrExtraTileRowsY for up tunnel components
        nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                m_height / tileSize - (nbrTilesY +
                     GameBoard::m_nbrTileRowsForStart +
                     GameBoard::m_nbrTileRowsForEnd)));
    }

    m_gameBoardStartRowColumn.first = GameBoard::m_nbrTileRowsForStart + nbrExtraTileRowsY/2;
    m_gameBoardStartRowColumn.second = nbrExtraTileRowsX/2;

    m_gameBoardEndRowColumn.first = GameBoard::m_nbrTileRowsForStart + nbrTilesY + nbrExtraTileRowsY/2 - 1;
    m_gameBoardEndRowColumn.second = nbrTilesX + nbrExtraTileRowsX/2 - 1;

    m_scaleBall = tileSize/2.0f/m_originalBallDiameter;
    m_gameBoard.initialize(
            tileSize,
            glm::vec3{0.0f, 0.0f, m_mazeFloorZ},
            nbrTilesY + nbrExtraTileRowsY + GameBoard::m_nbrTileRowsForStart + GameBoard::m_nbrTileRowsForEnd,
            nbrTilesX + nbrExtraTileRowsX);
    if (m_gameBoard.heightInTiles() == 0 || m_gameBoard.widthInTiles() == 0) {
        throw std::runtime_error("MovablePassage game board blocks not initialized properly");
    }

    // The main board area for constructing the tunnel
    for (uint32_t k = m_gameBoardStartRowColumn.first; k <= m_gameBoardEndRowColumn.first; k++) {
        for (uint32_t l = m_gameBoardStartRowColumn.second; l <= m_gameBoardEndRowColumn.second; l++) {
            m_gameBoard.block(k, l).setBlockType(GameBoardBlock::BlockType::onBoard);
        }
    }

    // the fixed tunnel through the places for extra tunnel pieces at the bottom.
    for (uint32_t k = GameBoard::m_nbrTileRowsForStart;
         k < GameBoard::m_nbrTileRowsForStart + nbrExtraTileRowsY / 2;
         k++)
    {
        auto comp = m_components[Component::ComponentType::straight];
        auto pos = comp->add(k, m_gameBoardStartRowColumn.second + startColumn, 0.0f, true);
        auto &block = m_gameBoard.block(k, m_gameBoardStartRowColumn.second + startColumn);
        block.setComponent(comp, pos);
        block.setBlockType(GameBoardBlock::BlockType::onBoard);
    }

    // the fixed tunnel through the places for extra tunnel pieces at the top.
    auto &comp = m_components[Component::ComponentType::straight];
    for (uint32_t k = m_gameBoardEndRowColumn.first + 1;
         k < m_gameBoard.heightInTiles() - GameBoard::m_nbrTileRowsForEnd;
         k++)
    {
        auto pos = comp->add(k, m_gameBoardStartRowColumn.second + endColumn, 0.0f, true);
        auto &block = m_gameBoard.block(k, m_gameBoardStartRowColumn.second + endColumn);
        block.setComponent(comp, pos);
        block.setBlockType(GameBoardBlock::BlockType::onBoard);
    }

    // The start
    for (uint32_t k = 0; k < GameBoard::m_nbrTileRowsForStart; k++) {
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
            Component::ComponentType type = Component::ComponentType::open;
            float rotation = 0.0f;
            if (k == 0) {
                if (l == 0) {
                    type = Component::ComponentType::closedCorner;
                } else if (l == m_gameBoard.widthInTiles() - 1) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(90.0f);
                } else {
                    type = Component::ComponentType::closedBottom;
                }
            } else if (k == GameBoard::m_nbrTileRowsForStart - 1) {
                if (l == 0) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(270.0f);
                } else if (l == m_gameBoard.widthInTiles() - 1) {
                    type = Component::ComponentType::closedCorner;
                    rotation = glm::radians(180.0f);
                } else if (l == m_gameBoardStartRowColumn.second + startColumn) {
                    type = Component::ComponentType::open;
                    rotation = 0.0f;
                } else {
                    type = Component::ComponentType::closedBottom;
                    rotation = glm::radians(180.0f);
                }
            } else if (l == 0) {
                type = Component::ComponentType::closedBottom;
                rotation = glm::radians(270.0f);
            } else if (l == m_gameBoard.widthInTiles() - 1) {
                type = Component::ComponentType::closedBottom;
                rotation = glm::radians(90.0f);
            }
            auto &compEnd = m_components[type];
            auto pos = compEnd->add(k, l, rotation, true);
            auto &block = m_gameBoard.block(k, l);
            block.setComponent(compEnd, pos);
            block.setBlockType(GameBoardBlock::BlockType::begin);
        }
    }

    // The end
    for (uint32_t k = m_gameBoard.heightInTiles() - GameBoard::m_nbrTileRowsForEnd;
         k < m_gameBoard.heightInTiles();
         k++)
    {
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
            auto &block = m_gameBoard.block(k, l);
            if (k == m_gameBoard.heightInTiles() - GameBoard::m_nbrTileRowsForEnd &&
                l == m_gameBoardStartRowColumn.second + endColumn)
            {
                block.setBlockType(GameBoardBlock::BlockType::end);
                m_columnEndPosition = l;
            } else {
                block.setBlockType(GameBoardBlock::endOffBoard);
            }
            // don't set a component - we detect the end by the block type
        }
    }

    for (auto &component : m_components) {
        component->setSize(tileSize);
    }
}

void MovablePassage::initDone() {
    if (m_nbrComponents == 0) {
        throw std::runtime_error("MovablePassage components not initialized properly");
    }

    // place the passage components in the off board sections
    auto it1 = m_components.begin();
    auto end1 = m_components.end();
    if (it1 == end1) {
        throw std::runtime_error("MovablePassage components not initialized properly");
    }

    auto it2 = (*it1)->placementsBegin();
    auto end2 = (*it1)->placementsEnd();
    uint32_t m = 0;
    bool finished = false;
    for (uint32_t k = 0; k < m_gameBoard.heightInTiles() && !finished; k++){
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); /* increment in loop */) {
            if (((*it1)->type() != Component::ComponentType::straight &&
                    (*it1)->type() != Component::ComponentType::tjunction &&
                    (*it1)->type() != Component::ComponentType::turn &&
                    (*it1)->type() != Component::ComponentType::crossjunction) || it2 == end2 ) {
                it1++;

                if (it1 == end1) {
                    finished = true;
                    break;
                }

                m = 0;
                it2 = (*it1)->placementsBegin();
                end2 = (*it1)->placementsEnd();
                continue;
            }
            auto &block = m_gameBoard.block(k,l);
            if (block.blockType() != GameBoardBlock::BlockType::offBoard || block.component() != nullptr) {
                l++;
                continue;
            }
            if (it2 != end2) {
                if (!it2->lockedIntoPlace()) {
                    block.setComponent(*it1, m);
                    it2->setRC(k, l);

                    m++;
                    l++;
                }
                it2++;
            }
        }
    }

    // add the rock segments that were requested of us.
    std::shared_ptr<Component> &rock = m_components[Component::ComponentType::noMovementRock];
    for (auto const &rc: m_addedRocks) {
        uint32_t row = rc.first + m_gameBoardStartRowColumn.first;
        uint32_t col = rc.second + m_gameBoardStartRowColumn.second;

        size_t placementIndex = rock->add(row, col, 0.0f, true);
        auto &block = m_gameBoard.block(row, col);
        block.setComponent(rock, placementIndex);
    }

    // add the dirt and rock placements for parts of the board that are not covered yet.
    // rock for the off board placements, dirt for the on board
    std::shared_ptr<Component> dirt = m_components[Component::ComponentType::noMovementDirt];
    for (uint32_t k = GameBoard::m_nbrTileRowsForStart; k < m_gameBoard.heightInTiles() - GameBoard::m_nbrTileRowsForEnd; k++) {
        for (uint32_t l = 0; l < m_gameBoard.widthInTiles(); l++) {
            auto &b = m_gameBoard.block(k,l);
            if (b.component() == nullptr) {
                if (b.blockType() == GameBoardBlock::BlockType::onBoard) {
                    dirt->add(k, l, 0.0f);
                    b.setComponent(dirt, dirt->nbrPlacements() - 1);
                } else {
                    rock->add(k, l, 0.0f);
                    b.setComponent(rock, rock->nbrPlacements() - 1);
                }
            }
        }
    }

    m_ballRow = 0;
    m_ballCol = m_gameBoard.widthInTiles() / 2;
    m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol);
    m_initDone = true;
}

Component::CellWall nextCellWall(
        Component::CellWall wall,
        size_t nbrRows,
        size_t nbrCols,
        size_t &ballRowNext,
        size_t &ballColNext)
{
    switch (wall) {
        case Component::CellWall::noWall:
            return Component::CellWall ::noWall;
        case Component::CellWall::wallRight:
            if (ballColNext != nbrCols - 1) {
                ballColNext++;
                return Component::CellWall::wallLeft;
            } else {
                return Component::CellWall::noWall;
            }
        case Component::CellWall::wallUp:
            if (ballRowNext != nbrRows - 1) {
                ballRowNext++;
                return Component::CellWall::wallDown;
            } else {
                return Component::CellWall::noWall;
            }
        case Component::CellWall::wallLeft:
            if (ballColNext != 0) {
                ballColNext--;
                return Component::CellWall::wallRight;
            } else {
                return Component::CellWall::noWall;
            }
        case Component::CellWall::wallDown:
            if (ballRowNext != 0) {
                ballRowNext--;
                return Component::CellWall::wallUp;
            } else {
                return Component::CellWall::noWall;
            }
    }
}

void confineBall(float cellSize, float &posFromCenter) {
    if (posFromCenter > cellSize/2) {
        posFromCenter = cellSize/2 - Level::m_floatErrorAmount;
    } else if (posFromCenter < -cellSize/2) {
        posFromCenter = cellSize/2 - Level::m_floatErrorAmount;
    }
}

bool MovablePassage::updateData() {
    if (!m_initDone) {
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
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
    while (timeDiff >= 0.0f) {
        nbrComputations ++;
        if (nbrComputations > 200) {
            break;
        }
        glm::vec3 posFromCenter = position - m_gameBoard.position(m_ballRow, m_ballCol);
        if (fabs(posFromCenter.x) >= m_gameBoard.blockSize()/2 + 0.001f ||
            fabs(posFromCenter.y) >= m_gameBoard.blockSize()/2 + 0.001f) {
            // shouldn't happen
            return drawingNecessary();
        }

        Component::checkForNextWallFunc checkforNextWall{
            [&](Component::CellWall wall1, Component::CellWall wall2)
                -> std::pair<bool, bool>
            {
                auto hasWallAt = [&](std::pair<size_t, size_t> rc, Component::CellWall cellWall) -> bool {
                    auto &b = m_gameBoard.block(rc.first, rc.second);
                    if (b.blockType() == GameBoardBlock::BlockType::offBoard) {
                        return true;
                    }
                    return b.component()->hasWallAt(cellWall, b.placementIndex());
                };

                size_t width = m_gameBoard.widthInTiles();
                size_t height = m_gameBoard.heightInTiles();
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

                auto &bOld = m_gameBoard.block(m_ballRow, m_ballCol);
                Component::CellWall wall1Actual = bOld.component()->actualWall(wall1, bOld.placementIndex());
                Component::CellWall wall2Actual = bOld.component()->actualWall(wall2, bOld.placementIndex());
                auto rc1 = nextRC(wall1Actual, m_ballRow, m_ballCol);
                auto rc2 = nextRC(wall2Actual, m_ballRow, m_ballCol);
                auto rcOld = std::make_pair(m_ballRow, m_ballCol);
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
        };
        auto &block = m_gameBoard.block(m_ballRow, m_ballCol);
        auto walls = block.component()->moveBallInCell(
                block.placementIndex(), posFromCenter, timeDiff, m_ball.velocity, ballRadius(), checkforNextWall);
        if (fabs(posFromCenter.x) >= m_gameBoard.blockSize()/2 + 0.001f &&
            fabs(posFromCenter.y) >= m_gameBoard.blockSize()/2 + 0.001f) {
            // shouldn't happen
            return drawingNecessary();
        }

        position = m_gameBoard.position(m_ballRow, m_ballCol) + posFromCenter;
        if (walls.first == Component::noWall && walls.second == Component::noWall) {
            m_ball.position = position;
            return drawingNecessary();
        }
        if (walls.first == Component::CellWall::wallLeft || walls.second == Component::CellWall::wallLeft) {
            m_ballCol--;
        } else if (walls.first == Component::CellWall::wallRight || walls.second == Component::CellWall::wallRight) {
            m_ballCol++;
        }
        if (walls.first == Component::CellWall::wallDown || walls.second == Component::CellWall::wallDown) {
            m_ballRow--;
        } else if (walls.first == Component::CellWall::wallUp || walls.second == Component::CellWall::wallUp) {
            m_ballRow++;
        }

        auto &nextBlock = m_gameBoard.block(m_ballRow, m_ballCol);
        if (nextBlock.blockType() == GameBoardBlock::BlockType::end) {
            // winning condition.  Set m_finished and return that drawing is necessary.
            m_finished = true;
            m_ball.position = position;
            return true;
        } else if (block.blockType() == GameBoardBlock::BlockType::begin &&
                   nextBlock.blockType() == GameBoardBlock::BlockType::begin) {
            // advance the ball into the next cell, but don't track its path
            m_ball.position = position;
            if (checkBallBorders(m_ball.position, m_ball.velocity)) {
                // shouldn't happen
                return drawingNecessary();
            }
            return  drawingNecessary();
        }

        // the ball advanced to the next cell.  Track the ball moving into the next component.
        Component::Placement &placement = nextBlock.component()->placement(
                nextBlock.placementIndex());
        Component::Placement &oldPlacement = block.component()->placement(
                block.placementIndex());
        if (placement.next().first != nullptr) {
            if (placement.next().first == block.component() &&
                placement.next().second == block.placementIndex()) {
                // We are going backwards.  Unblock block that we were at and move on.
                oldPlacement.next() = std::make_pair(nullptr, 0);
                oldPlacement.prev() = std::make_pair(nullptr, 0);
            } else {
                // We encountered a loop.  Unblock the entire loop
                std::shared_ptr<Component> nextComponent = nextBlock.component();
                size_t index = nextBlock.placementIndex();
                std::shared_ptr<Component> loopComponent = block.component();
                size_t loopIndex = block.placementIndex();
                while (loopComponent != nextComponent && loopIndex != index) {
                    auto tmp = loopComponent->placement(loopIndex).prev();
                    auto &loopPlacement = loopComponent->placement(loopIndex);
                    loopPlacement.prev() = std::make_pair(nullptr, 0);
                    loopPlacement.next() = std::make_pair(nullptr, 0);
                    loopComponent = tmp.first;
                    loopIndex = tmp.second;
                }
            }
        } else {
            // the ball is entering a new cell that is not in its path yet.
            placement.prev() = std::make_pair(block.component(), block.placementIndex());
            oldPlacement.next() = std::make_pair(nextBlock.component(),
                                                 nextBlock.placementIndex());
        }
    }

    m_ball.position = position;
    updateRotation(timeDiff);

    return drawingNecessary();
}

bool MovablePassage::updateStaticDrawObjects(DrawObjectTable &objs, TextureMap &textures) {
    if (!m_initDone || !objs.empty()) {
        return false;
    }

    glm::vec3 endScale = m_gameBoard.scaleEndObject();
    endScale.x /= m_modelSize;
    endScale.y /= m_modelSize;
    auto objEnd = std::make_shared<DrawObject>();
    auto objEndOffBoard = std::make_shared<DrawObject>();
    getQuad(objEnd->vertices, objEnd->indices);
    getQuad(objEndOffBoard->vertices, objEndOffBoard->indices);
    objEnd->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_componentTextureEnd);
    objEndOffBoard->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester, m_textureEndOffBoard);
    textures.insert(std::make_pair(objEnd->texture, std::shared_ptr<TextureData>()));
    textures.insert(std::make_pair(objEndOffBoard->texture, std::shared_ptr<TextureData>()));
    for (uint32_t col = 0; col < m_gameBoard.widthInTiles(); col++) {
        glm::vec3 endPosition = m_gameBoard.position(
                m_gameBoard.heightInTiles() - m_gameBoard.m_nbrTileRowsForEnd, col);
        if (col == m_columnEndPosition) {
            objEnd->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), endPosition) *
                                            glm::scale(glm::mat4(1.0f), endScale));
        } else {
            objEndOffBoard->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), endPosition) *
                                            glm::scale(glm::mat4(1.0f), endScale));
        }
    }
    // no components for these so we don't set the reference back to this obj.
    objs.push_back(std::make_pair(objEnd, std::shared_ptr<DrawObjectData>()));
    objs.push_back(std::make_pair(objEndOffBoard, std::shared_ptr<DrawObjectData>()));

    std::pair<std::vector<Vertex>, std::vector<uint32_t>> v;
    loadModel(m_gameRequester->getAssetStream(m_componentModels[Component::ComponentType::closedCorner]), v);
    auto objStartCorner = std::make_shared<DrawObject>();
    std::swap(objStartCorner->vertices, v.first);
    std::swap(objStartCorner->indices, v.second);
    objStartCorner->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::closedCorner]);
    textures.insert(std::make_pair(objStartCorner->texture, std::shared_ptr<TextureData>()));
    uint32_t refStartCorner = objs.size();
    m_components[Component::ComponentType::closedCorner]->setObjReference(
            Component::MovableType::staticObj, refStartCorner);
    objs.push_back(std::make_pair(objStartCorner, std::shared_ptr<DrawObjectData>()));

    loadModel(m_gameRequester->getAssetStream(m_componentModels[Component::ComponentType::closedBottom]), v);
    auto objStartSide = std::make_shared<DrawObject>();
    std::swap(objStartSide->vertices, v.first);
    std::swap(objStartSide->indices, v.second);
    objStartSide->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::closedBottom]);
    textures.insert(std::make_pair(objStartSide->texture, std::shared_ptr<TextureData>()));
    uint32_t refStartSide = objs.size();
    m_components[Component::ComponentType::closedBottom]->setObjReference(
            Component::MovableType::staticObj, refStartSide);
    objs.push_back(std::make_pair(objStartSide, std::shared_ptr<DrawObjectData>()));

    loadModel(m_gameRequester->getAssetStream(m_componentModels[Component::ComponentType::open]), v);
    auto objStartCenter = std::make_shared<DrawObject>();
    std::swap(objStartCenter->vertices, v.first);
    std::swap(objStartCenter->indices, v.second);
    objStartCenter->texture = std::make_shared<TextureDescriptionPath>(m_gameRequester,
            m_componentTextures[Component::ComponentType::open]);
    textures.insert(std::make_pair(objStartCenter->texture, std::shared_ptr<TextureData>()));
    uint32_t refStartCenter = objs.size();
    m_components[Component::ComponentType::open]->setObjReference(
            Component::MovableType::staticObj, refStartCenter);
    objs.push_back(std::make_pair(objStartCenter, std::shared_ptr<DrawObjectData>()));

    float scale = m_gameBoard.blockSize()/m_modelSize;
    bool done = false;
    for (uint32_t i = 0; i < m_gameBoard.heightInTiles() && !done; i++) {
        for (uint32_t j = 0; j < m_gameBoard.widthInTiles(); j++) {
            if (m_gameBoard.blockType(i,j) != GameBoardBlock::BlockType::begin) {
                done = true;
                break;
            }
            auto &component = m_gameBoard.block(i,j).component();
            size_t placementIndex = m_gameBoard.block(i,j).placementIndex();
            auto &placement = component->placement(placementIndex);
            switch (component->type()) {
            case Component::ComponentType::closedCorner:
                objStartCorner->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                break;
            case Component::ComponentType::closedBottom:
                objStartSide->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                break;
            case Component::ComponentType::open:
                objStartCenter->modelMatrices.push_back(glm::translate(glm::mat4(1.0f), m_gameBoard.position(i,j)) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), glm::vec3{0.0f, 0.0f, 1.0f}) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale)));
                break;
            default:
                break;
            }
        }
    }

    return true;
}

bool MovablePassage::updateDynamicDrawObjects(DrawObjectTable &objs, TextureMap &textures, bool &texturesChanged) {
    glm::vec3 zaxis{0.0f, 0.0f, 1.0f};
    float scaleBall = m_gameBoard.blockSize()/m_modelSize/2.0f;
    if (objs.empty()) {
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> v;
        for (auto const &component: m_components) {
            if (component->type() == Component::ComponentType::closedBottom ||
                component->type() == Component::ComponentType::closedCorner ||
                component->type() == Component::ComponentType::open)
            {
                // These components are handled as static draw objects.
                continue;
            }
            auto obj = std::make_shared<DrawObject>();
            if (m_componentModels[component->type()].empty()) {
                getQuad(obj->vertices, obj->indices, glm::vec3{0.0f, 0.0f, 1.0f});
            } else {
                loadModel(m_gameRequester->getAssetStream(m_componentModels[component->type()]), v);
                std::swap(v.first, obj->vertices);
                std::swap(v.second, obj->indices);
            }

            obj->texture = std::make_shared<TextureDescriptionPath>(
                    m_gameRequester, m_componentTextures[component->type()]);
            textures.emplace(obj->texture, std::shared_ptr<TextureData>());
            float size = component->componentSize()/m_modelSize;
            for (auto it = component->placementsBegin(); it != component->placementsEnd(); it++) {
                obj->modelMatrices.push_back(
                    glm::translate(glm::mat4(1.0f), m_gameBoard.position(it->row(), it->col())) *
                    glm::rotate(glm::mat4(1.0f), it->rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size}));
            }
            component->setObjReference(Component::MovableType::dynamicObj, objs.size());
            objs.push_back(std::make_pair(obj, std::shared_ptr<DrawObjectData>()));
        }

        // the ball
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
        for (size_t i = 0; i < objs.size(); i++) {
            auto &component = m_components[i];
            auto ref = component->objReference();
            if (component->type() == Component::ComponentType::closedBottom ||
                component->type() == Component::ComponentType::closedCorner ||
                component->type() == Component::ComponentType::open ||
                ref.first != Component::MovableType::dynamicObj)
            {
                // These components are handled as static draw objects.
                continue;
            }

            auto &obj = objs[ref.second].first;
            float size = component->componentSize()/m_modelSize;
            for (size_t j = 0; j <  component->nbrPlacements(); j++) {
                auto &placement = component->placement(j);
                obj->modelMatrices[j] =
                    glm::translate(glm::mat4(1.0f), m_gameBoard.position(placement.row(), placement.col())) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size});
            }
        }
        if (m_gameBoard.isMoveInProgress()) {
            auto rc = m_gameBoard.moveRC();
            auto &b = m_gameBoard.block(rc.first, rc.second);
            float size = b.component()->componentSize()/m_modelSize;
            auto &placement = b.component()->placement(b.placementIndex());
            glm::vec2 xy = placement.movePositionSoFar();
            auto ref = b.component()->objReference();
            if (ref.first != Component::MovableType::dynamicObj) {
                // shouldn't happen
                return true;
            }

            objs[ref.second].first->modelMatrices[b.placementIndex()] =
                    glm::translate(glm::mat4(1.0f), glm::vec3{xy.x, xy.y, m_zMovingPlacement}) *
                    glm::rotate(glm::mat4(1.0f), placement.rotationAngle(), zaxis) *
                    glm::scale(glm::mat4(1.0f), glm::vec3{size, size, size});
        }

        // the ball
        objs[m_objsReferenceBall].first->modelMatrices[0] =
                glm::translate(glm::mat4(1.0f), m_ball.position) *
                glm::mat4_cast(m_ball.totalRotated) *
                glm::scale(glm::mat4(1.0f), glm::vec3{scaleBall, scaleBall, scaleBall});

        texturesChanged = false;
    }

    return true;
}

bool MovablePassage::drag(float startX, float startY, float distanceX, float distanceY) {
    if (!m_initDone) {
        return false;
    }

    glm::vec2 startPosition{startX, startY};
    glm::vec2 distance{distanceX, distanceY};
    return m_gameBoard.drag(startPosition, distance);
}

bool MovablePassage::dragEnded(float x, float y) {
    glm::vec2 endPosition{x, y};
    return m_gameBoard.dragEnded(endPosition);
}

bool MovablePassage::tap(float x, float y) {
    glm::vec2 position{x, y};
    return m_gameBoard.tap(position);
}
