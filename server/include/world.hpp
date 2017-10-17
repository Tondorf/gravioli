#pragma once

#include "server.hpp"
#include "worldProperties.hpp"


namespace simulation {
    class World {
    protected:
        const WorldProperties _wprop;
        bool _stopped;

        virtual void loop(std::uint64_t);

    public:
        const int ID;

        World() = delete;

        World(int id, const WorldProperties&);

        World(const World&) = delete;

        World& operator=(const World&) = delete;

        virtual ~World() = default;
        
        virtual bool run();

        virtual void stop();
    };
}
