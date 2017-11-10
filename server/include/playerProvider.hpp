#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include "runnable.hpp"
#include "webclient.hpp"
namespace simulation { class Player; }


namespace simulation {
    class PlayerProvider: public utils::Runnable {
    protected:
        enum class PlayerDataStatus {
            AWAIT_NEW_DATA, NEW_DATA_READY
        } _playerDataStatus;

        std::mutex _mutex;

        utils::WebClient _webclient;

        std::vector<std::shared_ptr<Player>> _players;
        std::vector<std::shared_ptr<Player>> _updatedPlayers;

        virtual std::vector<int> getPlayerIDs();

        virtual std::shared_ptr<Player> getPlayerById(int id,
                                                      bool expect200OK = true);

    public:
        PlayerProvider();

        virtual ~PlayerProvider() = default;

        PlayerProvider(const PlayerProvider&) = delete;

        PlayerProvider& operator=(const PlayerProvider&) = delete;

        virtual bool loop() override;

        virtual void updatePlayers();

        virtual const std::vector<std::shared_ptr<Player>>& getPlayers();
    };
}
