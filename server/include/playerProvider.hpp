#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "player.hpp"
#include "runnable.hpp"
#include "webclient.hpp"


namespace simulation {
    class PlayerProvider: public utils::Runnable {
    private:
        enum class PlayerDataStatus {
            AWAIT_NEW_DATA, NEW_DATA_READY
        } _playerDataStatus;

        std::mutex _mutex;

        utils::WebClient _webclient;

        std::vector<std::shared_ptr<Player>> _players;
        std::vector<std::shared_ptr<Player>> _updatedPlayers;

        std::vector<int> getPlayerIDs();
        std::shared_ptr<Player> getPlayerById(int id);

    public:
        PlayerProvider();

        virtual ~PlayerProvider() = default;

        PlayerProvider(const PlayerProvider&) = delete;

        PlayerProvider& operator=(const PlayerProvider&) = delete;

        bool loop() override;

        void updatePlayers();

        const std::vector<std::shared_ptr<Player>>& getPlayers();
    };
}
