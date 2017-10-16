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


    void World::sleep() {
        std::this_thread::sleep_for(WORLD_SLEEP);
    }


    bool World::run() {
        while (!_stopped) {
            loop();
            sleep();
        }

        return true;
    }


    void World::loop() {
    }


    void World::stop() {
        _stopped = true;
    }
}
