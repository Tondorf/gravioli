#pragma once

#include <atomic>
#include <memory>

#include "server.hpp"
#include "worldProperties.hpp"


namespace simulation {
    class World {
    private:
        const WorldProperties _winfo;
        std::shared_ptr<server::IMsgQueue> _msgQueue;
        bool _stopped;
        int _lastAllocSize;

        void sleep();

        void sendWorldToMsgQueue();


    public:
        const int ID;

        static std::atomic<std::size_t> currentlyAllocatedMsgInstances;

        World() = delete;

        World(int id,
              const WorldProperties&,
              std::shared_ptr<server::IMsgQueue>);

        World(const World&) = delete;

        World& operator=(const World&) = delete;

        virtual ~World();
        
        static void init(std::vector<std::shared_ptr<World>>&,
                         const WorldProperties&,
                         std::shared_ptr<server::IMsgQueue>);

        bool run();

        void stop();
    };
}
