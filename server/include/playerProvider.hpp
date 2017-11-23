#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include "runnable.hpp"
#include "optional.hpp"
namespace server { class Player; }


namespace server {
    class PlayerProvider: public utils::Runnable {
    public:
        class WebClient {
        public:
            WebClient() = default;

            virtual ~WebClient() = default;

            struct Response {
                std::string content;
                int statusCode;
            };
            virtual Response get(const std::string& url) = 0;
        };

    protected:
        enum class BinaryPlayerDataStatus {
            AWAIT_NEW_DATA, NEW_DATA_READY
        };
        std::atomic<BinaryPlayerDataStatus> _playerDataStatus;

        std::mutex _mutex;

        std::shared_ptr<WebClient> _webclient;

        std::vector<std::shared_ptr<Player>> _players;
        std::vector<std::shared_ptr<Player>> _updatedPlayers;

        virtual std::vector<int> getPlayerIDs();

        virtual stdx::optional<std::shared_ptr<Player>> getPlayerById(int id);

    public:
        PlayerProvider(const std::shared_ptr<WebClient>&);

        virtual ~PlayerProvider() = default;

        PlayerProvider(const PlayerProvider&) = delete;

        PlayerProvider& operator=(const PlayerProvider&) = delete;

        virtual bool loop() override;

        virtual void updatePlayers();

        virtual const std::vector<std::shared_ptr<Player>>& getPlayers();
    };
}
