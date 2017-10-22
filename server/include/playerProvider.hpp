#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "player.hpp"


namespace simulation {
    class PlayerProvider {
    private:
        enum class PlayerDataStatus {
            AWAIT_NEW_DATA, NEW_DATA_READY
        } _playerDataStatus;

        std::mutex _mutex;

        bool _stopped;

        std::vector<std::shared_ptr<Player>> _players;
        std::vector<std::shared_ptr<Player>> _updatedPlayers;

    public:
        PlayerProvider();

        virtual ~PlayerProvider() = default;

        PlayerProvider(const PlayerProvider&) = delete;

        PlayerProvider& operator=(const PlayerProvider&) = delete;

        bool run();

        void stop();

        void updatePlayers();

        const std::vector<std::shared_ptr<Player>>& getPlayers();
    };
}
