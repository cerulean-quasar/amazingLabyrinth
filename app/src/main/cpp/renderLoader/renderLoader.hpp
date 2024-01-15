/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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

using LoadedRenderDetailsEntry = ReferenceCountedInteger<size_t>;

template <typename traits>
class RenderLoader {
public:
    typename traits::RenderDetailsReferenceType load(
            std::shared_ptr<GameRequester> const &gameRequester,
            renderDetails::Query const &query,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters)
    {
        auto &registrar = traits::getRenderDetailsRegistrar();
        unsigned int score = 0;
        size_t matchingEntryNumber = registrar.size();
        for (size_t entryNumber = 0; entryNumber < registrar.size(); entryNumber++) {
            unsigned int newScore = registrar[entryNumber].description.getMatchPotential(query);
            if (newScore > score) {
                score = newScore;
                matchingEntryNumber = entryNumber;
            }
        }

        if (matchingEntryNumber >= registrar.size()) {
            throw std::runtime_error("RenderDetails not registered.");
        }

        typename traits::RetrieveFcns functions = registrar[matchingEntryNumber].getFunctions();

        if (registrar[matchingEntryNumber].cacheEntry != nullptr)
        {
            for (auto it = m_loadedRenderDetails.begin(); it != m_loadedRenderDetails.end(); it++) {
                if (*it->get() == matchingEntryNumber) {
                    if (structuralChangeNeeded(registrar[matchingEntryNumber].cacheEntry, surfaceDetails)) {
                        m_loadedRenderDetails.erase(it);
                        break;
                    }

                    if (m_loadedRenderDetails.size() > m_nbrRenderDetailsToKeep/2) {
                        LoadedRenderDetailsEntry entry = *it;
                        m_loadedRenderDetails.erase(it);
                        m_loadedRenderDetails.push_front(entry);
                    }

                    return loadExisting(functions, gameRequester,
                                        registrar[matchingEntryNumber].cacheEntry,
                                        surfaceDetails, parameters);
                }
            }

            // Shouldn't happen, but we can fix it. Registrar of render details had a cached render
            // details instance, but there was no entry of this render details entry in the list of
            // cached render details.
            if (registrar[matchingEntryNumber].cacheEntry != nullptr) {
                m_loadedRenderDetails.push_front(getLoadedRenderDetailsEntry(matchingEntryNumber));
                return loadExisting(functions, gameRequester,
                                    registrar[matchingEntryNumber].cacheEntry,
                                    surfaceDetails, parameters);
            }
        }

        typename traits::RenderDetailsReferenceType renderDetailsRef =
                loadNew(functions, gameRequester, surfaceDetails, parameters);

        registrar[matchingEntryNumber].cacheEntry = renderDetailsRef.renderDetails;
        m_loadedRenderDetails.push_front(getLoadedRenderDetailsEntry(matchingEntryNumber));

        int numberCacheEntriesToRemove = m_loadedRenderDetails.size() - m_nbrRenderDetailsToKeep;
        if (numberCacheEntriesToRemove > 0) {
            m_loadedRenderDetails.resize(m_nbrRenderDetailsToKeep);
        }

        return std::move(renderDetailsRef);
    };

    virtual ~RenderLoader() = default;

protected:
    static size_t constexpr m_nbrRenderDetailsToKeep = 5;
    std::list<LoadedRenderDetailsEntry> m_loadedRenderDetails;

    virtual typename traits::RenderDetailsReferenceType loadNew(
            typename traits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;
    virtual bool structuralChangeNeeded(
            std::shared_ptr<typename traits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails) = 0;
    virtual typename traits::RenderDetailsReferenceType loadExisting(
            typename traits::RetrieveFcns const &fcns,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<typename traits::RenderDetailsType> const &renderDetails,
            std::shared_ptr<typename traits::SurfaceDetailsType> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parameters) = 0;
private:
    typename traits::RetrieveFcns getFcns(renderDetails::Description const &desc) {
        auto loaderFcnIt = traits::getRenderDetailsMap().find(desc);
        if (loaderFcnIt == traits::getRenderDetailsMap().end()) {
            throw std::runtime_error("RenderDetails not registered.");
        }
        return loaderFcnIt->second();
    }

    LoadedRenderDetailsEntry getLoadedRenderDetailsEntry(size_t entryIndex) {
        auto deleter = [](size_t entryIndex) -> void {
            auto &registrar = traits::getRenderDetailsRegistrar();
            registrar[entryIndex].cacheEntry = nullptr;
        };

        return LoadedRenderDetailsEntry(entryIndex, deleter);
    }
};

#endif // AMAZING_LABYRINTH_RENDER_LOADER_HPP
