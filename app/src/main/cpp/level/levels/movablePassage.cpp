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

void MovablePassage::initSetGameBoard(
        uint32_t nbrTilesX,
        uint32_t nbrTilesY) {
    float tileSizeX = m_width / m_nbrTilesX;
    float tileSizeY = m_height / (m_nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                  GameBoard::m_nbrTileRowsForEnd);
    float tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;
    uint32_t nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
            (m_width - tileSize * m_nbrTilesX) / tileSize));
    uint32_t nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesY;

    // minus 2 * nbrExtraTileRowsY for up tunnel components
    uint32_t nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
            (m_height - tileSize * m_nbrTilesY) / tileSize));
    uint32_t nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;

    // we have a device with a screen of a similar size to our tunnel building area,
    // get some more space by making the movable space less.
    if (nbrExtraTilesX + nbrExtraTilesY < m_nbrComponents) {
        uint32_t moreExtraComponentsRequired = m_nbrComponents - (nbrExtraTilesX + nbrExtraTilesY);
        uint32_t nbrExtraPerimeters =
                moreExtraComponentsRequired / (2 * m_nbrTilesX + 2 * m_nbrTilesY) + 1;
        tileSizeX = m_width / (m_nbrTilesX + 2 * nbrExtraPerimeters);
        tileSizeY = m_height / (m_nbrTilesY + GameBoard::m_nbrTileRowsForStart +
                                GameBoard::m_nbrTileRowsForEnd + 2 * nbrExtraPerimeters);
        tileSize = tileSizeX < tileSizeY ? tileSizeX : tileSizeY;

        nbrExtraTileRowsX = static_cast<uint32_t>(std::floor(
                (m_width - tileSize * m_nbrTilesX) / tileSize));
        nbrExtraTilesX = nbrExtraTileRowsX * m_nbrTilesX;

        // minus 2 * nbrExtraTileRowsY for up tunnel components
        nbrExtraTileRowsY = static_cast<uint32_t>(std::floor(
                (m_height - tileSize * m_nbrTilesY) / tileSize));
        nbrExtraTilesY = nbrExtraTileRowsY * m_nbrTilesX - 2 * nbrExtraTileRowsY;
    }

    m_gameBoard.initialize(
            (m_nbrTilesX + nbrExtraTileRowsX) * tileSize,
            (m_nbrTilesY + nbrExtraTileRowsY + GameBoard::m_nbrTileRowsForEnd +
             GameBoard::m_nbrTileRowsForStart) * tileSize,
            glm::vec3{0.0f, 0.0f, 0.0f},
            m_nbrTilesY + nbrExtraTileRowsY, m_nbrTilesX + nbrExtraTileRowsX);
    if (m_gameBoard.heightInTiles() == 0 || m_gameBoard.widthInTiles() == 0) {
        throw std::runtime_error("MovablePassage game board blocks not initialized properly");
    }

    // The main board area for constructing the tunnel
    for (uint32_t k = nbrExtraTileRowsX / 2; k < m_nbrTilesX - nbrExtraTileRowsX; k++) {
        for (uint32_t l = nbrExtraTileRowsY / 2; l < m_nbrTilesY - nbrExtraTileRowsY; l++) {
            m_gameBoard.block(k, l).setBlockType(GameBoardBlock::BlockType::onBoard);
        }
    }

    uint32_t col = (m_nbrTilesX + nbrExtraTileRowsX) / 2;
    // the fixed tunnel through the places for extra tunnel pieces at the bottom.
    for (uint32_t k = GameBoard::m_nbrTileRowsForStart; k < nbrExtraTileRowsY / 2; k++) {
        auto &comp = m_components[Component::ComponentType::straight];
        auto pos = comp->add(k, col, 0.0f, true);
        auto &block = m_gameBoard.block(k, col);
        block.setComponent(comp, pos);
        block.setBlockType(GameBoardBlock::BlockType::onBoard);
    }

    // the fixed tunnel through the places for extra tunnel pieces at the top.
    auto &comp = m_components[Component::ComponentType::straight];
    for (uint32_t k = m_nbrTilesY - nbrExtraTileRowsY / 2 + GameBoard::m_nbrTileRowsForStart;
         k < m_nbrTilesY + nbrExtraTileRowsY + GameBoard::m_nbrTileRowsForStart;
         k++)
    {
        auto pos = comp->add(k, col, 0.0f, true);
        auto &block = m_gameBoard.block(k, col);
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
            auto &block = m_gameBoard.block(0, l);
            block.setBlockType(GameBoardBlock::BlockType::end);
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
    auto it2 = (*it1)->placementsBegin();
    auto end2 = (*it1)->placementsEnd();
    uint32_t m = 0;
    bool finished = false;
    for (uint32_t k = 0; k < m_gameBoard.widthInTiles() && !finished; k++){
        for (uint32_t l = 0; l < m_gameBoard.heightInTiles(); l++) {
            auto &block = m_gameBoard.block(k,l);
            if (it2 != end2 && block.blockType() == GameBoardBlock::BlockType::offBoard && block.component() == nullptr) {
                block.setComponent(*it1, m);
                it2->m_row = k;
                it2->m_col = l;

                it2++;
                m++;
            }
            if (it2 == end2) {
                it1++;

                if (it1 == end1) {
                    finished = true;
                    break;
                }

                m = 0;
                it2 = (*it1)->placementsBegin();
                end2 = (*it1)->placementsEnd();
            }
        }
    }

    m_ballRow = 0;
    m_ballCol = m_gameBoard.widthInTiles() / 2;
    m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol);
}

bool MovablePassage::updateData() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_prevTime).count();
    m_prevTime = currentTime;

    glm::vec3 position = m_ball.position;
    glm::vec3 prevPosition = position;
    glm::vec3 velocity = getUpdatedVelocity(m_ball.acceleration, timeDiff);
    while (timeDiff > 0.0f) {
        glm::vec3 posFromCenter = m_gameBoard.position(m_ballRow, m_ballCol) - position;
        auto &block = m_gameBoard.block(m_ballRow, m_ballCol);
        Component::CellWall wall = block.component()->moveBallInCell(
                block.placementIndex(), posFromCenter, timeDiff, velocity);
        size_t ballRowNext = m_ballRow;
        size_t ballColNext = m_ballCol;
        Component::CellWall wallNextCell;
        switch (wall) {
        case Component::CellWall::noWall:
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            return drawingNecessary();
        case Component::CellWall::wallRight:
            if (ballRowNext != m_gameBoard.widthInTiles() - 1) {
                ballRowNext++;
                wallNextCell = Component::CellWall::wallLeft;
            }
            break;
        case Component::CellWall::wallUp:
            if (ballColNext != m_gameBoard.heightInTiles() - 1) {
                ballColNext++;
                wallNextCell = Component::CellWall::wallDown;
            }
            break;
        case Component::CellWall::wallLeft:
            if (ballRowNext != 0) {
                ballRowNext--;
                wallNextCell = Component::CellWall::wallRight;
            }
            break;
        case Component::CellWall::wallDown:
            if (ballColNext != 0) {
                ballColNext--;
                wallNextCell = Component::CellWall::wallUp;
            }
        }

        // the ball is rolling off the game board area - move to the edge and return
        if (m_ballRow == ballRowNext && m_ballCol == ballColNext) {
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return drawingNecessary();
        }

        auto &nextBlock = m_gameBoard.block(ballRowNext, ballColNext);
        if (nextBlock.blockType() == GameBoardBlock::BlockType::end) {
            // winning condition.  Set m_finished and return that drawing is necessary.
            m_finished = true;
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            return true;
        } else if (nextBlock.blockType() == GameBoardBlock::BlockType::offBoard ||
                   nextBlock.component() == nullptr) {
            // hit the edge of the game board or there is no component for the ball to
            // move into, stop the ball at the edge of the cell and return
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ball.velocity = {0.0f, 0.0f, 0.0f};
            return drawingNecessary();
        } else if (block.blockType() == GameBoardBlock::BlockType::begin &&
                   nextBlock.blockType() == GameBoardBlock::BlockType::begin) {
            // advance the ball into the next cell, but don't track its path
            m_ball.position = m_gameBoard.position(m_ballRow, m_ballCol) - posFromCenter;
            m_ball.prevPosition = prevPosition;
            m_ballCol = ballColNext;
            m_ballRow = ballRowNext;
            continue;
        }

        if (!nextBlock.component()->hasWallAt(wallNextCell, nextBlock.placementIndex())) {
            // ball allowed to advance into next door cell.  set flag to notify caller that
            // the textures have changed.  Also track the ball moving into the next component.
            m_texturesChanged = true;
            Component::Placement &placement = nextBlock.component()->placement(
                    nextBlock.placementIndex());
            Component::Placement &oldPlacement = block.component()->placement(
                    block.placementIndex());
            if (placement.m_next.first != nullptr) {
                if (placement.m_next.first == block.component() &&
                    placement.m_next.second == block.placementIndex()) {
                    // We are going backwards.  Unblock block that we were at and move on.
                    oldPlacement.m_next = std::make_pair(nullptr, 0);
                    oldPlacement.m_prev = std::make_pair(nullptr, 0);
                } else {
                    // We encountered a loop.  Unblock the entire loop
                    std::shared_ptr<Component> nextComponent = nextBlock.component();
                    size_t index = nextBlock.placementIndex();
                    std::shared_ptr<Component> loopComponent = block.component();
                    size_t loopIndex = block.placementIndex();
                    while (loopComponent != nextComponent && loopIndex != index) {
                        auto tmp = loopComponent->placement(loopIndex).m_prev;
                        auto &loopPlacement = loopComponent->placement(loopIndex);
                        loopPlacement.m_prev = std::make_pair(nullptr, 0);
                        loopPlacement.m_next = std::make_pair(nullptr, 0);
                        loopComponent = tmp.first;
                        loopIndex = tmp.second;
                    }
                }
            } else {
                // the ball is entering a new cell that is not in its path yet.
                placement.m_prev = std::make_pair(block.component(), block.placementIndex());
                oldPlacement.m_next = std::make_pair(nextBlock.component(),
                                                     nextBlock.placementIndex());
            }
            m_ballCol = ballColNext;
            m_ballRow = ballRowNext;
        }
    }
}