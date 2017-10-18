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


    void World::init(std::uint64_t) {
        auto newPlanet = [](double x, double y, double z) {
            Vec3d pos{x, y, z};
            Planet p;
            p.pos = pos;
            return p;
        };

        _planets.clear();
        _planets.push_back(newPlanet(0, 0, 0));
    }


    void World::loop(std::uint64_t t) {
        const auto n = _planets.size();

        constexpr std::size_t x = 0;
        constexpr std::size_t y = 1;
        constexpr std::size_t z = 2;
        for (std::size_t i = 0; i < n; ++i) {
            auto phase = t / 2. * pi / 1000.;
            phase += static_cast<double>(i) * pi / 2.1;

            _planets[i].pos[x] = std::cos(phase);
            _planets[i].pos[y] = std::sin(phase);
            _planets[i].pos[z] = std::sin(phase + pi / 4.);
        }
    }
}
