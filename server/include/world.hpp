#pragma once

#include <cstdint>
#include <memory>
namespace server { class PlayerProvider; }


namespace server {
    class World {
    protected:
        std::shared_ptr<PlayerProvider> _playerProvider;
        bool _stopped;

        virtual void loop(std::uint64_t) = 0;

        virtual void sendWorldToMsgQueue() = 0;

    public:
        const int ID;

        World() = delete;

        World(int id, std::shared_ptr<PlayerProvider>);

        World(const World&) = delete;

        World& operator=(const World&) = delete;

        virtual ~World() = default;
        
        virtual bool run();

        virtual void stop();
    };
}
