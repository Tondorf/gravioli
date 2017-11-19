#include "playerProvider.hpp"
#include <cstddef>
#include <exception>
#include <string>
#include <thread>
#include "SimpleLogger/logger.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include "json/json.hpp"
#include "player.hpp"


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

        try {
            using json = nlohmann::json;
            auto j = json::parse(jsonString);

            if (auto jsonIDs = j["IDs"]; jsonIDs.is_array()) {
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
        } catch (const std::exception& e) {
            Log::error("Error during JSON parsing: %s", e.what());
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

        try {
            using json = nlohmann::json;
            auto j = json::parse(jsonString);

            if (auto jsonKey = j["key"];
                jsonKey.is_array() && jsonKey.size() == crypto::KEY_BLOCKSIZE) {
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
        } catch (const std::exception& e) {
            Log::error("Error during JSON parsing: %s", e.what());
        }

        return nullptr;
    }


    void PlayerProvider::updatePlayers() {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_playerDataStatus != PlayerDataStatus::AWAIT_NEW_DATA) {
                /*
                 *     x != PlayerDataStatus::AWAIT_NEW_DATA
                 * <=> x == PlayerDataStatus::NEW_DATA_READY
                 *
                 * There are already new data available that wait
                 * to get pulled via PlayerProvider::getPlayers().
                 * Returning at this point is not an error, though this 
                 * might indicate an unnecessary call to this method.
                 *
                 * This is some messed up variation of caching, thus returning
                 * 'false' (or even 'true') is misleading since there
                 * actually was a successful update.  
                 */
                return;
            }
        }

        _updatedPlayers.clear();
        for (const auto& playerID : getPlayerIDs()) {
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
