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
#ifndef AMAZING_LABYRINTH_RENDER_LOADER_HPP
#define AMAZING_LABYRINTH_RENDER_LOADER_HPP

#include <memory>

#include "../renderDetails/basic/renderDetailsData.hpp"

class RenderLoader {
public:
    std::shared_ptr<RenderDetailsData> load(std::shared_ptr<RenderDetails> const &renderDetails,
            uint32_t width, uint32_t height)
    {
        for (auto it = m_loadedRenderDetailsData.begin(); it != m_loadedRenderDetailsData.end(); it++) {
            if (it->name() == renderDetails.name()) {
                auto renderDetailsData = *it;
                if (renderDetailsData.width() != width || renderDetailsData.height() != height) {
                    reload(renderDetailsData);
                }
                if (m_loadedRenderDetailsData.size() > m_nbrRenderDetailsDataToKeep) {
                    m_loadedRenderDetailsData.erase(it);
                    m_loadedRenderDetailsData.push_front(renderDetailsData);
                }
                return std::move(renderDetailsData);
            }
        }

        auto renderDetailsData = loadNew(renderDetails->name());
        m_loadedRenderDetailsData.push_front(renderDetailsData);
        while (m_loadedRenderDetailsData.size() > m_nbrRenderDetailsDataToKeep) {
            m_loadedRenderDetailsData.pop_back();
        }

        return renderDetailsData;
    };

    virtual ~RenderLoader() = default;

protected:
    virtual std::shared_ptr<RenderDetailsData> loadNew(std::string const &name, uint32_t width, uint32_t height) = 0;
    virtual void reload(std::shared_ptr<RenderDetailsData> const &renderDetailsData) = 0;
private:
    static size_t constexpr m_nbrRenderDetailsDataToKeep = 10;
    std::list<std::shared_ptr<RenderDetailsData>> m_loadedRenderDetailsData;
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_HPP
