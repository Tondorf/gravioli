#pragma once

#include <atomic>
#include <memory>

#include "server.hpp"
#include "worldInfoProvider.hpp"


namespace simulation {
    class World {
    private:
        const WorldInfoProvider _winfo;
        std::shared_ptr<server::IMsgQueue> _msgQueue;
        bool _stopped;
        int _lastAllocSize;

        void sleep();

        void sendWorldToMsgQueue();


    public:
        const int ID;

        static std::atomic<std::size_t> currentlyAllocatedFBInstances;

        World(int id,
              const WorldInfoProvider&,
              std::shared_ptr<server::IMsgQueue>);

        virtual ~World();
        
        static void init(std::vector<World>&,
                         const WorldInfoProvider&,
                         std::shared_ptr<server::IMsgQueue>);

        bool run();

        void stop();
    };
}
