#pragma once

#include "server.hpp"
#include "worldProperties.hpp"


namespace simulation {
    using Vec3d = std::array<double, 3>;

    struct Planet {
        Vec3d pos;
    };

    class World {
    protected:
        const WorldProperties _wprop;
        bool _stopped;

        std::vector<Planet> _planets;

        virtual void init(std::uint64_t);

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
