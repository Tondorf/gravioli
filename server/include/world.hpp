#pragma once

#include <memory>

#include "server.hpp"
#include "worldInfoProvider.hpp"


namespace simulation {
    class World {
    private:
        const WorldInfoProvider _winfo;
        std::shared_ptr<server::IMsgQueue> _msgQueue;


    public:
        const int ID;

        World(int id,
              const WorldInfoProvider&,
              std::shared_ptr<server::IMsgQueue>);

        virtual ~World() = default;
        
        static void init(std::vector<World>&,
                         const WorldInfoProvider&,
                         std::shared_ptr<server::IMsgQueue>);

        bool run();

        void stop();
    };
}
