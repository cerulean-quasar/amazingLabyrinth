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
#include <list>
#include <string>

#include "../renderDetails/renderDetails.hpp"

template <typename traits>
class RenderLoader {
public:
    typename traits::RenderDetailsReferenceType load(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::string const &name,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters)
    {
        typename traits::RetrieveFcns fcns = getFcns(name);
        for (auto it = m_loadedRenderDetails.begin(); it != m_loadedRenderDetails.end(); it++) {
            if ((*it)->nameString() == name) {
                auto renderDetails = *it;
                if (renderDetails->width() != surfaceDetails->surfaceWidth ||
                    renderDetails->height() != surfaceDetails->surfaceHeight)
                {
                    reload(gameRequester, renderDetails, surfaceDetails, parameters);
                }

                if (m_loadedRenderDetails.size() > m_nbrRenderDetailsToKeep/2) {
                    m_loadedRenderDetails.erase(it);
                    m_loadedRenderDetails.push_front(renderDetails);
                }

                return loadExisting(fcns, gameRequester, renderDetails, surfaceDetails, parameters);
            }
        }

        typename traits::RenderDetailsReferenceType renderDetailsRef =
                loadNew(fcns, gameRequester, parameters);
        m_loadedRenderDetails.push_front(renderDetailsRef.renderDetails);
        while (m_loadedRenderDetails.size() > m_nbrRenderDetailsToKeep) {
            m_loadedRenderDetails.pop_back();
        }

        return std::move(renderDetailsRef);
    };

    virtual ~RenderLoader() = default;

protected:
    static size_t constexpr m_nbrRenderDetailsToKeep = 10;
    std::list<std::shared_ptr<typename traits::RenderDetailsType>> m_loadedRenderDetails;

    virtual typename traits::RenderDetailsReferenceType loadNew(
            typename traits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;
    virtual void reload(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<typename traits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails) = 0;
    virtual typename traits::RenderDetailsReferenceType loadExisting(
            typename traits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<typename traits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;
private:
    typename traits::RetrieveFcns getFcns(std::string const &name) {
        auto loaderFcnIt = traits::getRenderDetailsMap().find(name);
        if (loaderFcnIt == traits::getRenderDetailsMap().end()) {
            throw std::runtime_error("RenderDetails not registered.");
        }
        return loaderFcnIt->second();
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_HPP
