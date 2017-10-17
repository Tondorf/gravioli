#include <thread>

#include "config.hpp"
#include "server.hpp"

#include "world.hpp"


namespace simulation {
    World::World(int id, const WorldProperties& wprop) :
        _wprop(wprop),
        _stopped(false),
        ID(id) {
    }


    bool World::run() {
        std::uint64_t ticks = 0;
        while (!_stopped) {
            auto start = std::chrono::steady_clock::now();

            loop(ticks);
            ticks += 1;

            auto end = std::chrono::steady_clock::now();
            auto dt = end - start;
            std::this_thread::sleep_for(WORLD_SLEEP - dt);
        }

        return true;
    }


    void World::loop(std::uint64_t) {
    }


    void World::stop() {
        _stopped = true;
    }
}
