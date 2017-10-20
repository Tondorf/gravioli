#pragma once

#include <memory>
#include <random>

#include "user.hpp"
#include "server.hpp"
#include "worldProperties.hpp"

#include "flatbuffers/flatbuffers.h"


namespace simulation {
    using Vec3d = std::array<double, 3>;

    struct Planet {
        Vec3d pos;
    };

    class World {
    private:
        std::random_device _rnddev;
        std::mt19937 _rndgen;

        std::vector<Planet> _planets;

    protected:
        std::shared_ptr<WorldProperties> _wprop;
        bool _stopped;

        virtual void init();

        virtual void loop(std::uint64_t);

        virtual void serializeWorldForUser(std::shared_ptr<User>,
                                           flatbuffers::FlatBufferBuilder *);

    public:
        const int ID;

        World() = delete;

        World(int id, std::shared_ptr<WorldProperties>);

        World(const World&) = delete;

        World& operator=(const World&) = delete;

        virtual ~World() = default;
        
        virtual bool run();

        virtual void stop();
    };
}
