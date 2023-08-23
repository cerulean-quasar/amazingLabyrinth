#ifndef AMAZING_LABYRINTH_TESTZ_SERIALIZER_HPP
#define AMAZING_LABYRINTH_TESTZ_SERIALIZER_HPP

#include <json.hpp>

#include "loadData.hpp"

namespace testZ {
    void to_json(nlohmann::json &j, LevelSaveData const &val);

    void from_json(nlohmann::json const &j, LevelSaveData &val);

    void to_json(nlohmann::json &j, LevelConfigData const &val);

    void from_json(nlohmann::json const &j, LevelConfigData &val);
} // namespace testZ

#endif // AMAZING_LABYRINTH_TESTZ_SERIALIZER_HPP