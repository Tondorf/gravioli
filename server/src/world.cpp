#include <thread>

#include "SimpleLogger/logger.hpp"

#include "config.hpp"
#include "server.hpp"

#include "world.hpp"


namespace simulation {
    World::World(int id, const WorldProperties& wprop) :
        _wprop(wprop),
        _stopped(false),
        ID(id) {

        init();
    }


    bool World::run() {
        std::uint64_t ticks = 0;
        while (!_stopped) {
            auto start = std::chrono::high_resolution_clock::now();

            loop(ticks);
            ticks += 1;

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> dt = end - start;

            std::chrono::duration<double, std::milli> sleep = WORLD_SLEEP - dt;
            if (sleep.count() < 0) {
                Log::error("World #%d: loop evaluation is too time consuming! "
                           "Last iteration took %.0f ms.", ID, dt.count());
            } else {
                std::this_thread::sleep_for(sleep);
            }
        }

        return true;
    }


    void World::stop() {
        _stopped = true;
    }


    void World::init() {
        auto newPlanet = [](double x, double y, double z) {
            Vec3d pos{x, y, z};
            Planet p;
            p.pos = pos;
            return p;
        };

        _planets.clear();
        _planets.push_back(newPlanet(0, 0, 0));
        _planets.push_back(newPlanet(0, 0, 0));
    }


    void World::loop(std::uint64_t t) {
        const auto n = _planets.size();

        constexpr std::size_t x = 0;
        constexpr std::size_t y = 1;
        constexpr std::size_t z = 2;
        for (std::size_t i = 0; i < n; ++i) {
            auto phase1 = static_cast<double>(t) / 2. * pi / 1000.;
            auto phase2 = static_cast<double>(t) / 2. * pi / 2000.;
            auto phase3 = static_cast<double>(t) / 2. * pi / 3000.;

            _planets[i].pos[x] = std::sin(phase1) + static_cast<double>(2 * i);
            _planets[i].pos[y] = std::sin(phase2);
            _planets[i].pos[z] = std::sin(phase3);
        }
    }
}
