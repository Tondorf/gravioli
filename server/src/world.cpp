#include "server.hpp"

#include "world.hpp"


namespace simulation {
    World::World(int id,
                 const WorldInfoProvider& winfo,
                 std::shared_ptr<server::IMsgQueue> msgQueue) :
        _winfo(winfo),
        _msgQueue(msgQueue),
        ID(id) {
    }


    void World::init(std::vector<World>& worlds,
                     const WorldInfoProvider& winfo,
                     std::shared_ptr<server::IMsgQueue> msgQueue) {
        worlds.push_back(World(0, winfo, msgQueue));
    }


    bool World::run() {
        return true;
    }


    void World::stop() {
    }
}
