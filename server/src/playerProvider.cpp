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
    PlayerProvider::PlayerProvider(const std::shared_ptr<WebClient>& webClient):
        _playerDataStatus(BinaryPlayerDataStatus::AWAIT_NEW_DATA),
        _webclient(webClient) {
    }


    bool PlayerProvider::loop() {
        updatePlayers();

        std::this_thread::sleep_for(USER_SYNC_INTERVAL);

        return true;
    }


    std::vector<int> PlayerProvider::getPlayerIDs() {
        const auto res = _webclient->get("players");

        std::vector<int> playerIDs;
        if (res.statusCode != 200) {
            Log::error("WebClient returned status code: %d. "
                       "This is an error!", res.statusCode);
            return playerIDs;
        }

        try {
            using json = nlohmann::json;
            auto j = json::parse(res.content);

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


    stdx::optional<std::shared_ptr<Player>> PlayerProvider::getPlayerById(
        int id) {

        const auto url = std::string("player/") + std::to_string(id);
        const auto res = _webclient->get(url);

        if (res.statusCode != 200 || res.content.empty()) {
            return stdx::nullopt;
        }

        try {
            using json = nlohmann::json;
            auto j = json::parse(res.content);

            if (auto jsonKey = j["key"];
                jsonKey.is_array() && jsonKey.size() == crypto::KEY_BLOCKSIZE) {
                crypto::Key key;
                for (std::size_t i = 0; i < crypto::KEY_BLOCKSIZE; ++i) {
                    auto b = jsonKey[i].get<int>();
                    if (b >= 0 && b <= 255) {
                        key[i] = static_cast<byte>(b);
                    } else {
                        Log::error("Digit of key is not of type byte");
                        return stdx::nullopt;
                    }
                }
                return std::make_shared<Player>(id, key);
            } else {
                Log::error("JSON is malformed!");
            }
        } catch (const std::exception& e) {
            Log::error("Error during JSON parsing: %s", e.what());
        }

        return stdx::nullopt;
    }


    /*
     * !!! WARNING !!!
     *
     * Make sure to read ALL (!) comments below and to be 100% sure, that you
     * understand the current logic of this method and its dependency with
     *
     * PlayerProvider::getPlayers()
     *
     * before doing any changes! There are assumptions that are not forced
     * by design, i.e. your changes might compile but this does not guarantee
     * thread-safety!
     */
    void PlayerProvider::updatePlayers() {
        if (_playerDataStatus != BinaryPlayerDataStatus::AWAIT_NEW_DATA) {
            /*
             *     x != BinaryPlayerDataStatus::AWAIT_NEW_DATA
             * <=> x == BinaryPlayerDataStatus::NEW_DATA_READY
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
        
        /*
         * Reaching this point guarantees:
         *
         * _playerDataStatus == BinaryPlayerDataStatus::AWAIT_NEW_DATA
         *
         * Thus, _updatedPlayers will never get touched as long as
         * _playerDataStatus is not set to
         *
         * BinaryPlayerDataStatus::NEW_DATA_READY
         *
         * and this is done in this method only!
         */

        _updatedPlayers.clear();
        for (const auto& playerID : getPlayerIDs()) {
            auto&& player = getPlayerById(playerID);
            if (player) {
                _updatedPlayers.push_back(player.value());
            } else {
                Log::error("Could not find Player by ID = %d", playerID);
            }
        }

        /*
         * This unleashes PlayerProvider::getPlayers()...
         */
        _playerDataStatus = BinaryPlayerDataStatus::NEW_DATA_READY;
    }


    const std::vector<std::shared_ptr<Player>>& PlayerProvider::getPlayers() {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_playerDataStatus == BinaryPlayerDataStatus::NEW_DATA_READY) {
                std::swap(_players, _updatedPlayers);

                _playerDataStatus = BinaryPlayerDataStatus::AWAIT_NEW_DATA;
            }
        }
        
        return _players;
    }
}
