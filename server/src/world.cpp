#include "world.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <future>
#include <ratio>
#include <thread>
#include "SimpleLogger/logger.hpp"
#include "config.hpp"
#include "flatbuffers/flatbuffers.h"
#include "playerProvider.hpp"
#include "world_generated.h"
namespace simulation { class Player; }


namespace simulation {
    World::World(int id, std::shared_ptr<PlayerProvider> playerProvider) :
        _playerProvider(playerProvider),
        _stopped(false),
        ID(id) {
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

            sendWorldToMsgQueue();

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
}
