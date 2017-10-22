#include <future>
#include <thread>

#include "SimpleLogger/logger.hpp"

#include "world_generated.h"

#include "config.hpp"
#include "server.hpp"

#include "world.hpp"


namespace simulation {
    World::World(int id, std::shared_ptr<PlayerProvider> playerProvider) :
        _rndgen(_rnddev()),
        _playerProvider(playerProvider),
        _stopped(false),
        ID(id) {

        init();
    }


    bool World::run() {
        auto playerProviderExitedSuccessfully = std::async(std::launch::async,
            [playerProvider=_playerProvider]() {
                return playerProvider->run();
            }
        );

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

        bool playerProviderExitStatus = playerProviderExitedSuccessfully.get();
        if (playerProviderExitStatus) {
            Log::info("Stopped player provider successfully.");
        } else {
            Log::error("Player provider exited with errors!");
        }

        return playerProviderExitStatus;
    }


    void World::stop() {
        _playerProvider->stop();

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


    void World::serializeWorldForPlayer(
        std::shared_ptr<Player> /*player*/,
        flatbuffers::FlatBufferBuilder *builder) {

        std::vector<flatbuffers::Offset<game::Planet>> planets;
        planets.reserve(_planets.size());
        for (auto&& p: _planets) {
            auto x = static_cast<float>(p.pos[0]);
            auto y = static_cast<float>(p.pos[1]);
            auto z = static_cast<float>(p.pos[2]);
            planets.push_back(
                game::CreatePlanet(*builder, x, y, z)
            );
        }
        shuffle(planets.begin(), planets.end(), _rndgen);

        auto game = game::CreatePlanets(
            *builder,
            builder->CreateVector(planets)
        );

        builder->Finish(game);
    }
}
