#include <thread>

#include "config.hpp"
#include "server.hpp"

#include "world.hpp"


namespace simulation {
    World::World(int id,
                 const WorldInfoProvider& winfo,
                 std::shared_ptr<server::IMsgQueue> msgQueue) :
        _winfo(winfo),
        _msgQueue(msgQueue),
        _stopped(false),
        ID(id) {
    }


    void World::init(std::vector<World>& worlds,
                     const WorldInfoProvider& winfo,
                     std::shared_ptr<server::IMsgQueue> msgQueue) {
        worlds.push_back(World(0, winfo, msgQueue));
    }


    void World::sleep() {
        std::this_thread::sleep_for(WORLD_SLEEP);
    }


    bool World::run() {
        while (!_stopped) {
            sleep();
        }

        return true;
    }


    void World::stop() {
        _stopped = true;
    }
}
