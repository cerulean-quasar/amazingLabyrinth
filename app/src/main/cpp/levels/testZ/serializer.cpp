#include <memory>
#include <boost/implicit_cast.hpp>
#include <json.hpp>

#include "level.hpp"
#include "../../levelTracker/internals.hpp"
#include "../basic/loadData.hpp"
#include "../basic/level.hpp"
#include "loadData.hpp"
#include "../basic/serializer.hpp"
#include "serializer.hpp"

namespace testZ {
    void to_json(nlohmann::json &j, LevelSaveData const &val) {
        to_json(j, boost::implicit_cast<basic::LevelSaveData const &>(val));
    }

    void from_json(nlohmann::json const &j, LevelSaveData &val) {
        from_json(j, boost::implicit_cast<basic::LevelSaveData &>(val));
    }

    std::vector<uint8_t> Level::saveData(levelTracker::GameSaveData const &gsd,
                                         char const *saveLevelDataKey) {
        LevelSaveData sd;
        nlohmann::json j;
        to_json(j, gsd);
        j[saveLevelDataKey] = sd;
        return nlohmann::json::to_cbor(j);
    }

    levelTracker::Register<levelTracker::LevelMapTable, levelTracker::levelTable, basic::LevelConfigData, LevelSaveData, Level> registerLevel;
} // namespace testZ
