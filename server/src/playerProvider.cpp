#include <thread>

#include "json/json.hpp"
#include "SimpleLogger/logger.hpp"

#include "playerProvider.hpp"


namespace simulation {
    PlayerProvider::PlayerProvider():
        _playerDataStatus(PlayerDataStatus::AWAIT_NEW_DATA) {
    }


    bool PlayerProvider::loop() {
        updatePlayers();

        std::this_thread::sleep_for(USER_SYNC_SLEEP);

        return true;
    }


    std::vector<int> PlayerProvider::getPlayerIDs() {
        int statusCode;
        auto jsonString = _webclient.get("players", statusCode);

        std::vector<int> playerIDs;
        if (statusCode != 200) {
            Log::error("WebClient returned status code: %d. "
                       "This is an error!", statusCode);
            return playerIDs;
        }

        using json = nlohmann::json;
        auto j = json::parse(jsonString);

        auto jsonIDs = j["IDs"];
        if (jsonIDs.is_array()) {
            playerIDs.reserve(jsonIDs.size());
            for (auto id : jsonIDs) {
                if (id.is_number()) {
                    playerIDs.push_back(id.get<int>());
                } else {
                    Log::error("ID is not a number!");
                }
            }
        } else {
            Log::error("JSON element with id \"key\" is not an array!");
        }

        return playerIDs;
    }


    std::shared_ptr<Player> PlayerProvider::getPlayerById(int id,
                                                          bool expect200OK) {
        const auto url = std::string("player/") + std::to_string(id);

        int statusCode;
        auto jsonString = _webclient.get(url, statusCode);

        if (expect200OK && statusCode != 200) {
            Log::error("WebClient returned status code: %d. "
                       "This is an error!", statusCode);
            return nullptr;
        }

        if (jsonString.empty()) {
            return nullptr;
        }

        using json = nlohmann::json;
        auto j = json::parse(jsonString);

        auto jsonKey = j["key"];
        if (jsonKey.is_array() && jsonKey.size() == crypto::KEY_BLOCKSIZE) {
            crypto::Key key;
            for (std::size_t i = 0; i < crypto::KEY_BLOCKSIZE; ++i) {
                auto b = jsonKey[i].get<int>();
                if (b >= 0 && b <= 255) {
                    key[i] = static_cast<byte>(b);
                } else {
                    Log::error("Digit of key is not of type byte");
                    return nullptr;
                }
            }

            return std::make_shared<Player>(id, key);
        } else {
            Log::error("JSON is malformed!");
        }

        return nullptr;
    }


    void PlayerProvider::updatePlayers() {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_playerDataStatus != PlayerDataStatus::AWAIT_NEW_DATA) {
                return;
            }
        }

        _updatedPlayers.clear();
        for (auto playerID : getPlayerIDs()) {
            auto&& player = getPlayerById(playerID);
            if (player != nullptr) {
                _updatedPlayers.push_back(player);
            }
        }

        _playerDataStatus = PlayerDataStatus::NEW_DATA_READY;
    }


    const std::vector<std::shared_ptr<Player>>& PlayerProvider::getPlayers() {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_playerDataStatus == PlayerDataStatus::NEW_DATA_READY) {
                std::swap(_players, _updatedPlayers);

                _playerDataStatus = PlayerDataStatus::AWAIT_NEW_DATA;
            }
        }
        
        return _players;
    }
}
