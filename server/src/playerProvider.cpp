#include <thread>

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


    void PlayerProvider::updatePlayers() {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_playerDataStatus != PlayerDataStatus::AWAIT_NEW_DATA) {
                return;
            }
        }

        _updatedPlayers.clear();
        _updatedPlayers.push_back(std::make_shared<Player>(0, crypto::Key {
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f
        }));
        _updatedPlayers.push_back(std::make_shared<Player>(1, crypto::Key {
            0x01, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f
        }));

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
